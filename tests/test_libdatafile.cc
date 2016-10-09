/*! \file test_libdatafile.cc
 *
 * Implementation of test code for libdatafile.
 *
 * (C) 2016 Benjamin Naecker bnaecker@stanford.edu
 */

#include "test_libdatafile.h"

#include <vector>

void DatafileTest::initTestCase()
{
	/* Create data */
	m_data.set_size(datafile::BLOCK_SIZE * 3, datafile::NUM_CHANNELS);
	m_data.fill(arma::fill::randu);
	m_hidensData.set_size(datafile::BLOCK_SIZE * 3, hidensfile::NUM_CHANNELS);
	m_data.fill(arma::fill::randu);

	/* Names for the files */
	m_datafileName = "test-datafile.h5";
	m_hidensfileName = "test-hidensfile.h5";
	m_snipfileName = "test-snipfile.snip";
	m_hidensSnipfileName = "test-hidenssnipfile.snip";

	/* Remove them first if they exist */
	if (QFile::exists(m_datafileName)) {
		QFile::remove(m_datafileName);
	}
	if (QFile::exists(m_hidensfileName)) {
		QFile::remove(m_hidensfileName);
	}
	if (QFile::exists(m_snipfileName)) {
		QFile::remove(m_snipfileName);
	}
	if (QFile::exists(m_hidensSnipfileName)) {
		QFile::remove(m_hidensSnipfileName);
	}

	/* Create the files */
	m_dataFile.reset(new DataFile(m_datafileName.toStdString()));
	m_hidensFile.reset(new HidensFile(m_hidensfileName.toStdString()));
}

void DatafileTest::cleanupTestCase()
{
	/* Remove files */
	if (QFile::exists(m_datafileName)) {
		QFile::remove(m_datafileName);
	}
	if (QFile::exists(m_hidensfileName)) {
		QFile::remove(m_hidensfileName);
	}
	if (QFile::exists(m_snipfileName)) {
		QFile::remove(m_snipfileName);
	}
	if (QFile::exists(m_hidensSnipfileName)) {
		QFile::remove(m_hidensSnipfileName);
	}
}

void DatafileTest::testBadFile()
{
	QString filename = "./test_libdatafile.cc";
	DataFile* tmp = nullptr;
	QVERIFY_EXCEPTION_THROWN(tmp = new DataFile(filename.toStdString()), std::invalid_argument);
	delete tmp;
}

void DatafileTest::testBasicAttributes()
{
	m_dataFile->setGain(1.);
	QVERIFY2(m_dataFile->gain() == 1.,
			"ADC gain was set or read incorrectly");

	m_dataFile->setOffset(-5.0);
	QVERIFY2(m_dataFile->offset() == -5.0,
			"ADC offset was set or read incorrectly");

	auto now = QDateTime::currentDateTime().toString().toStdString();
	m_dataFile->setDate(now);
	QVERIFY2(m_dataFile->date() == now,
			"Date was set or read incorrectly");

	m_dataFile->setAnalogOutputSize(100);
	QVERIFY2(m_dataFile->analogOutputSize() == 100,
			"Size of analog output not correctly set.");

	QString array { "hexagonal" };
	m_dataFile->setArray(array.toStdString());
	QVERIFY2(m_dataFile->array() == "hexagonal",
			"Array type was set or read incorrectly");

	QVERIFY2(m_dataFile->filename() == m_datafileName.toStdString(),
			"Filename was set or read incorrectly.");

	QVERIFY2(m_dataFile->nchannels() == datafile::NUM_CHANNELS,
			"Number of channels for base DataFile class set or read incorrectly.");
	QVERIFY2(m_hidensFile->nchannels() == hidensfile::NUM_CHANNELS,
			"Number of channels for HidensFile class set or read incorrectly.");

	QVERIFY2(m_dataFile->nsamples() == 0,
			"Number of samples should be 0 before any data is written.");

	QVERIFY2(m_dataFile->length() == 0.,
			"Length of the data file should be zero before any data is written.");
}

void DatafileTest::testSmallDataReadWrite()
{
	/* Write small chunk of data. */
	int subsetSize = 100;
	auto subset = m_data.rows(0, subsetSize - 1);
	m_dataFile->setData(0, subsetSize, subset);

	/* Verify that the number of samples and lenth have changed. */
	QVERIFY2(m_dataFile->nsamples() == subsetSize,
			"Number of samples not updated correctly when new data is added.");
	QVERIFY2(m_dataFile->length() == 
			static_cast<double>(subsetSize) / static_cast<double>(m_dataFile->sampleRate()),
			"Length of data in seconds not updated correctly when new data is added.");

	/* Verify that reading back the newly-created analog output works. */
	auto aout = m_dataFile->analogOutput();
	QVERIFY2(static_cast<int>(aout.size()) == m_dataFile->analogOutputSize(),
			"Size of returned analog output does not match.");
	QVERIFY2(arma::all(arma::conv_to<arma::Col<qint16>>::from(aout) == subset.col(1)),
			"Analog output not correctly returned.");

	/* Verify that reading a single channel back works */
	auto v = m_dataFile->data(0, 0, subsetSize);
	QVERIFY2(arma::all(arma::conv_to<arma::Col<qint16>>::from(v) == subset.col(0)), 
			"Data from single channel not written or read correctly.");

	/* Verify that reading the full chunk back works */
	decltype(m_data) read;
	m_dataFile->data(0, m_dataFile->nchannels(), 0, subsetSize, read);
	QVERIFY2(arma::all(arma::vectorise(subset == read)),
			"Data read into arma::Mat<int16_t>& not read correctly.");

	/* Verify that reading the full chunk as raw voltage values works. */
	auto volts = m_dataFile->data(0, subsetSize);
	QVERIFY2(arma::all(arma::vectorise(
			((volts - m_dataFile->offset()) / m_dataFile->gain()) == subset)),
			"Data read as true voltage values not generated correctly.");
}

void DatafileTest::testReadPastEnd()
{
	/* Verify that reading data past the end of the file fails. */
	int subsetSize = 100;
	QVERIFY_EXCEPTION_THROWN(m_dataFile->data(0, 2 * subsetSize),
			std::logic_error);
}

void DatafileTest::testExtendFile()
{
	/* Write the full chunk of data. This should cause an extension of 
	 * the data file by at least a full block.
	 */
	m_dataFile->setData(0, m_data.n_rows, m_data);
	
	/* Verify the size has changed. */
	QVERIFY2(m_dataFile->nsamples() == static_cast<int>(m_data.n_rows),
			"Number of samples not updated correctly when new data added.");
	QVERIFY2(m_dataFile->length() == 
			static_cast<double>(m_data.n_rows) / static_cast<double>(m_dataFile->sampleRate()),
			"Length of data in seconds not updated correctly when new data is added.");

	/* Read back data, and verify write succeeded. */
	decltype(m_data) read;
	m_dataFile->data(0, m_dataFile->nchannels(), 0, 
			m_dataFile->nsamples(), read);
	QVERIFY2((read.size() == m_data.size()) && arma::all(arma::vectorise(read == m_data)),
			"Error when adding data that should extend the size of the file.");
}

void DatafileTest::testWriteToExistingFile()
{
	/* Close the data file, which causes flush of all data and attributes. */
	m_dataFile.reset(nullptr);

	/* Reopen the file, which should now be read-only and so the write
	 * should fail.
	 */
	DataFile df(m_datafileName.toStdString());
	QVERIFY_EXCEPTION_THROWN(df.setData(0, 100, m_data.rows(0, 100)),
			std::logic_error);
}

bool configsEqual(const Configuration& c1, const Configuration& c2)
{
	if (c1.size() != c2.size())
		return false;

	for (decltype(c1.size()) i = 0; i < c1.size(); i++) {
		auto& e1 = c1[i];
		auto& e2 = c2[i];
		if ( (e1.index != e2.index) ||
				(e1.xpos != e2.xpos) ||
				(e1.x != e2.x) ||
				(e1.ypos != e2.ypos) ||
				(e1.y != e2.y) ||
				(e1.label != e2.label) ) {
			return false;
		}
	}
	return true;
}

void DatafileTest::testConfiguration()
{
	/* Create random configuration */
	int nchannels = m_hidensData.n_cols;
	arma::Col<quint32> 
		index(nchannels, arma::fill::randu), 
		xpos(nchannels, arma::fill::randu), 
		ypos(nchannels, arma::fill::randu);
	arma::Col<quint16> 
		x(nchannels, arma::fill::randu), 
		y(nchannels, arma::fill::randu);
	arma::Col<quint8> label(nchannels, arma::fill::randu);

	m_config.resize(nchannels);
	for (auto i = 0; i < nchannels; i++) {
		m_config[i] = Electrode{ index(i), xpos(i), x(i), ypos(i), y(i), label(i) };
	}

	/* Write the configuration. */
	m_hidensFile->setConfiguration(m_config);

	/* Read the configuration and verify they're the same. */
	auto config = m_hidensFile->configuration();
	QVERIFY(configsEqual(config, m_config));
}

void DatafileTest::testCreateSnippetFiles()
{
	/* First write data to hidens file (not written yet.) */
	m_hidensFile->setData(0, m_hidensData.n_rows, m_hidensData);
	m_hidensFile->setArray("hidens");
	m_hidensFile->setGain(1.0);
	m_hidensFile->setOffset(-1.0);

	/* Re-open the original data file */
	m_dataFile.reset(new DataFile(m_datafileName.toStdString()));

	/* Create the data files */
	try {
		m_snipFile.reset(new SnipFile(m_snipfileName.toStdString(), *m_dataFile.get()));
		m_hidensSnipfile.reset(new HidensSnipFile(m_hidensSnipfileName.toStdString(), *m_hidensFile.get()));
	} catch ( ... ) {
		QFAIL("An exception was thrown creating snippet files from the data files.");
	}
}

void DatafileTest::testSourceInformationCopied()
{
	QVERIFY2(m_dataFile->array() == m_snipFile->array(),
			"Snippet file did not correctly copy the array type.");
	QVERIFY2(m_dataFile->nsamples() == static_cast<int>(m_snipFile->nsamples()),
			"Snippet file did not correctly copy the number of data samples.");
	QVERIFY2(m_dataFile->date() == m_snipFile->date(),
			"Snippet file did not correctly copy the date.");
	QVERIFY2(m_dataFile->gain() == m_snipFile->gain(),
			"Snippet file did not correctly copy the ADC gain.");
	QVERIFY2(m_dataFile->offset() == m_snipFile->offset(),
			"Snippet file did not correctly copy the ADC offset.");

	/* Read the configuration from the HiDens snippet file. */
	Configuration config;
	auto label = m_hidensSnipfile->label();
	auto xpos = m_hidensSnipfile->xpos();
	auto x = m_hidensSnipfile->x();
	auto ypos = m_hidensSnipfile->ypos();
	auto y = m_hidensSnipfile->y();
	auto ix = m_hidensSnipfile->indices();
	for (auto i = 0; i < static_cast<int>(m_config.size()); i++) {
		config.push_back( Electrode{ ix(i), xpos(i), x(i), ypos(i), y(i), label(i) } );
	}
	QVERIFY2(configsEqual(m_config, config), 
			"Snippet file did not correctly copy the Hidens configuration.");
}

void DatafileTest::testReadWriteChannels()
{
	arma::uvec channels(m_data.n_cols);
	for (auto i = 0; i < static_cast<int>(m_data.n_cols); i++)
		channels(i) = i;
	m_snipFile->setChannels(channels);

	auto chans = m_snipFile->channels();
	QVERIFY2(arma::all(chans == channels),
			"Snippet file extracted channels written or read incorrectly.");
}

void DatafileTest::testReadWriteThresholds()
{
	arma::vec thresh(m_data.n_cols, arma::fill::randn);
	m_snipFile->setThresholds(thresh);
	QVERIFY2(arma::all(m_snipFile->thresholds() == thresh),
			"Snippet file thresholds written or read incorrectly.");
}

bool snippetsEqual(
		const std::vector<arma::uvec>& idx1, const std::vector<arma::Mat<qint16>>& snip1,
		const std::vector<arma::uvec>& idx2, const std::vector<arma::Mat<qint16>>& snip2)
{
	if ( (idx1.size() != idx2.size()) || (snip1.size() != snip2.size()) || 
			(idx1.size() != snip1.size()) ) {
		return false;
	}

	for (decltype(idx1.size()) i = 0; i < idx1.size(); i++) {
		if ( (idx1[i].size() != idx2[i].size()) ||
				(snip1[i].size() != snip2[i].size()) ) {
			return false;
		}
		if (arma::any(idx1[i] != idx2[i])) {
			return false;
		}
		if (arma::any(arma::vectorise(snip1[i] != snip2[i]))) {
			return false;
		}
	}
	return true;
}

void DatafileTest::testReadWriteSnippets()
{
	/* Create snippets */
	std::vector<arma::Mat<qint16>> spikeSnips, noiseSnips;
	std::vector<arma::uvec> spikeIdx, noiseIdx;
	int nsnips = 100;
	int snipsize = m_snipFile->nsamplesBefore() + m_snipFile->nsamplesAfter() + 1;
	for (auto i = 0; i < m_dataFile->nchannels(); i++) {
		spikeSnips.push_back(arma::Mat<qint16>(nsnips, snipsize, arma::fill::randu));
		spikeIdx.push_back(arma::uvec(snipsize, arma::fill::randu));

		noiseSnips.push_back(arma::Mat<qint16>(nsnips, snipsize, arma::fill::randu));
		noiseIdx.push_back(arma::uvec(snipsize, arma::fill::randu));
	}

	/* Write them to the file */
	m_snipFile->writeSpikeSnips(spikeIdx, spikeSnips);
	m_snipFile->writeNoiseSnips(noiseIdx, noiseSnips);

	decltype(spikeSnips) readSpikeSnips, readNoiseSnips;
	decltype(spikeIdx) readSpikeIdx, readNoiseIdx;

	/* Read them back. Note that this actually tests both overloads of the write*Snips
	 * functions, as the one reading all snippets at once is defined by using
	 * the overload that reads a single channel at a time.
	 */
	m_snipFile->spikeSnips(readSpikeIdx, readSpikeSnips);
	m_snipFile->noiseSnips(readNoiseIdx, readNoiseSnips);

	/* Verify all snippets are equal */
	QVERIFY2(snippetsEqual(spikeIdx, spikeSnips, readSpikeIdx, readSpikeSnips),
			"Reading/writing spike snippets failed.");
	QVERIFY2(snippetsEqual(noiseIdx, noiseSnips, readNoiseIdx, readNoiseSnips),
			"Reading/writing noise snippets failed.");
}

void DatafileTest::testReadWriteMeans()
{
	arma::vec means(m_dataFile->nchannels(), arma::fill::randn);
	m_dataFile->setMeans(means);

	QVERIFY2(arma::all(m_dataFile->means() == means),
			"Channel mean values not correctly read or written.");
}

QTEST_APPLESS_MAIN(DatafileTest)
