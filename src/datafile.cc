/* h5recording.cpp
 * Implementation of class representing a single MEA recording and
 * the HDF5 file to which it is saved
 */

#include <sys/stat.h>
#include <iostream>
#include <ctime>

#include "datafile.h"

namespace datafile {

DataFile::DataFile(const std::string& filename, 
		const std::string& array,
		const hsize_t nchannels)
		: m_filename(filename),
		  m_array(array),
		  m_date("unknown"),
		  m_room("unknown"),
		  m_nsamples(0),
		  m_aoutSize(0)
{
	/* Turn off automatic printing of errors */
	H5::Exception::dontPrint();

	/* If file exists, verify it is valid HDF5 and load data from it.
	 * Else, construct a new file.
	 */
	struct stat buffer;
	if (stat(m_filename.c_str(), &buffer) == 0) {
		if (!H5::H5File::isHdf5(m_filename)) {
			std::cerr << "Invalid H5 file" << std::endl;
			throw std::invalid_argument("Invalid HDF5 file");
		}
		try {
			m_readOnly = true;
			m_file = H5::H5File(m_filename, H5F_ACC_RDWR); // must be read-write so we can call setMeans()
		} catch (H5::FileIException &e) {
			std::cerr << "Could not open H5 file" << std::endl;
			throw std::runtime_error("Could not open HDF5 file");
		}

		/* Open the dataset */
		try {
			m_dataset = m_file.openDataSet("data");
		} catch (H5::FileIException &e) {
			std::cerr << "File must contain a dataset labeled 'data'";
			throw std::invalid_argument("File must contain a 'data' dataset");
		}
		m_dataspace = m_dataset.getSpace();
		m_datatype = m_dataset.getDataType();

		/* Read attributes into data members */
		try {
			readArray();
			readSampleRate();
			readGain();
			readOffset();
			readDate();
			readRoom();
			readNumSamples();
			readAnalogOutputSize();
		} catch ( ... ) {
			std::cerr << "File does not contain appropriate attributes";
			throw std::invalid_argument(
					"H5 file does not contain appropriate attributes");
		}

	} else {
		/* Construct the file. Define to have a chunk cache large enough to hold
		 * a few chunks at a time.
		 */
		m_readOnly = false;
		H5::FileAccPropList m_fileProps(H5::FileAccPropList::DEFAULT);
		int mdc_nelmts = 0;
		size_t rdcc_nelmts, rdcc_nbytes = 0;
		size_t chunkCacheSizeElems = (
				datafile::ChunkCacheSize * datafile::BlockSize * 
				datafile::NumChannels
			);
		double rdcc_w0 = 0.0;
		m_fileProps.getCache(mdc_nelmts, rdcc_nelmts, rdcc_nbytes, rdcc_w0);
		m_fileProps.setCache(mdc_nelmts, chunkCacheSizeElems, 
				chunkCacheSizeElems * sizeof(int16_t), rdcc_w0);
		m_file = H5::H5File(m_filename, H5F_ACC_TRUNC, 
				H5::FileCreatPropList::DEFAULT, m_fileProps);

		/* Create the dataset */
		hsize_t dims[DatasetRank] = {nchannels, DatasetDefaultDims[1]};
		m_dataspace = H5::DataSpace(DatasetRank, dims, DatasetMaxDims);
		m_props = H5::DSetCreatPropList();
		m_props.setChunk(DatasetRank, DatasetChunkDims);
		m_datatype = H5::DataType(H5::PredType::STD_I16LE);
		m_dataset = m_file.createDataSet("data", m_datatype, m_dataspace, m_props);

		/* Set default parameters */
		setSampleRate(SampleRate);
		setRoom(DefaultRoomString);
		setArray(m_array);
	}
}

DataFile::~DataFile() 
{
	try {
		if (!readOnly()) 
			writeAllAttributes();
		m_file.close();
	} catch (H5::FileIException &e) {
		std::cerr << "Error closing HDF5 file: " << m_filename << std::endl;
	}
}

std::string DataFile::filename() const { return m_filename; }

std::string DataFile::array() const { return m_array; }

double DataFile::length() const 
{ 
	return ((double) nsamples() / sampleRate());
}

int DataFile::nsamples() const
{
	return static_cast<int>(m_nsamples);
}

int DataFile::nchannels() const
{
	hsize_t dims[DatasetRank] = {0, 0};
	m_dataspace.getSimpleExtentDims(dims);
	return dims[0];
}

float DataFile::sampleRate(void) const { return m_sampleRate; }

float DataFile::gain(void) const { return m_gain; }

float DataFile::offset(void) const { return m_offset; }

std::string DataFile::date(void) const { return m_date; }

std::string DataFile::room(void) const { return m_room; }

samples DataFile::data(int startSample, int endSample) const
{
	samples s;
	data(0, nchannels(), startSample, endSample, s);
	return s * gain();
}

arma::vec DataFile::data(int channel, int startSample, int endSample) const
{
	arma::vec s;
	data(channel, channel + 1, startSample, endSample, s);
	return s * gain();
}

void DataFile::verifyReadRequest(int startChannel, int endChannel, 
		int startSample, int endSample) const
{
	if ( (startSample < 0) || (startSample > nsamples()) ) {
		throw std::logic_error("Requested start sample out of range: " + 
				std::to_string(startSample) + " is not in range [0, " +
				std::to_string(nsamples()) + "]");
	}
	if ( (endSample < 0) || (endSample > nsamples()) ) {
		throw std::logic_error("Requested end sample out of range: " + 
				std::to_string(endSample) + " is not in range [0, " +
				std::to_string(nsamples()) + "]");
	}
	int requestedSamples = endSample - startSample;
	if (requestedSamples <= 0) {
		throw std::logic_error("Requested sample range invalid: (" + 
				std::to_string(startSample) + " - " + 
				std::to_string(endSample) + ")");
	}

	if ( (startChannel < 0) || (startChannel > nchannels()) ) {
		throw std::logic_error("Requested start channel out of range: " + 
				std::to_string(startSample) + " is not in range [0, " +
				std::to_string(nsamples()) + "]");
	}
	if ( (endChannel < 0) || (endChannel > nchannels()) ) {
		throw std::logic_error("Requested end channel out of range: " + 
				std::to_string(startSample) + " is not in range [0, " +
				std::to_string(nsamples()) + "]");
	}
	int requestedChannels = endChannel - startChannel;
	if (requestedChannels <= 0) {
		throw std::logic_error("Requested sample range invalid: (" + 
				std::to_string(startSample) + "-" + 
				std::to_string(endSample) + ")");
	}
}

H5::DataSpace DataFile::setupRead(int startChannel, int endChannel, 
		int startSample, int endSample) const
{
	int requestedSamples = endSample - startSample;
	int requestedChannels = endChannel - startChannel;

	/* Compute the source file data space */
	hsize_t fileOffset[DatasetRank] = {
			static_cast<hsize_t>(startChannel), 
			static_cast<hsize_t>(startSample)
		};
	hsize_t fileCount[DatasetRank] = {
			static_cast<hsize_t>(requestedChannels),
			static_cast<hsize_t>(requestedSamples)
		};
	m_dataspace.selectHyperslab(H5S_SELECT_SET, fileCount, fileOffset);
	if (!m_dataspace.selectValid()) {
		std::stringstream what;
		what << "Dataset selection invalid:" << std::endl
				<< "Offset: (" << startSample << ", 0)" << std::endl
				<< "Count: (" << requestedSamples << ", "
				<< nchannels() << ")" << std::endl;
		throw std::logic_error(what.str());
	}

	/* Define the destination data space in memory */
	hsize_t dims[DatasetRank] = {
			static_cast<hsize_t>(requestedChannels),
			static_cast<hsize_t>(requestedSamples)
		};
	hsize_t memOffset[DatasetRank] = {0, 0};
	hsize_t memCount[DatasetRank] = {
			static_cast<hsize_t>(requestedChannels),
			static_cast<hsize_t>(requestedSamples)
		};
	H5::DataSpace memspace{DatasetRank, dims};
	memspace.selectHyperslab(H5S_SELECT_SET, memCount, memOffset);
	if (!memspace.selectValid()) {
		std::stringstream what;
		what << "Memory dataspace selection invalid:" << std::endl
				<< "Count: (" << requestedSamples << ", "
				<< nchannels() << ")" << std::endl;
		throw std::logic_error(what.str());
	}
	return memspace;
}

void DataFile::writeDataAttr(const std::string& name, const H5::DataType &type, void *buf) 
{
	if (readOnly())
		return;
	try {
		H5::DataType writeType(type);
		if (!(m_dataset.attrExists(name))) {
			H5::DataSpace space(H5S_SCALAR);
			m_dataset.createAttribute(name, writeType, space);
		}
		H5::Attribute attr = m_dataset.openAttribute(name);
		attr.write(writeType, buf);
	} catch (H5::DataSetIException &e) {
		std::cerr << "DataSet exception accessing: " << name << std::endl;
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	}
}

void DataFile::writeDataStringAttr(const std::string& name, const std::string& value) 
{
	if ( (readOnly()) || (value.length() == 0) )
		return;
	try {
		H5::StrType stringType(0, value.length());
		if (!(m_dataset.attrExists(name))) {
			H5::DataSpace space(H5S_SCALAR);
			m_dataset.createAttribute(name, stringType, space);
		}
		H5::Attribute attr = m_dataset.openAttribute(name);
		attr.write(stringType, value.c_str());
	} catch (H5::DataSetIException &e) {
		std::cerr << "DataSet exception accessing: " << name << std::endl;
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	}
}

void DataFile::writeAllAttributes(void) 
{
	writeDataAttr("sample-rate", H5::PredType::IEEE_F32LE, &m_sampleRate);
	writeDataAttr("gain", H5::PredType::IEEE_F32LE, &m_gain);
	writeDataAttr("offset", H5::PredType::IEEE_F32LE, &m_offset);
	writeDataAttr("nsamples", H5::PredType::STD_U64LE, &m_nsamples);
	writeDataStringAttr("array", m_array);
	writeDataStringAttr("date", m_date);
	writeDataStringAttr("room", m_room);
}

void DataFile::setSampleRate(float sampleRate) 
{
	writeDataAttr("sample-rate", H5::PredType::IEEE_F32LE, &sampleRate);
	m_sampleRate = sampleRate;
}

void DataFile::setGain(float gain) 
{
	writeDataAttr("gain", H5::PredType::IEEE_F32LE, &gain);
	m_gain = gain;
}

void DataFile::setOffset(float offset) 
{
	writeDataAttr("offset", H5::PredType::IEEE_F32LE, &offset);
	m_offset = offset;
}

void DataFile::setDate(std::string date)
{
	writeDataStringAttr("date", date);
	m_date = date;
}

void DataFile::setRoom(std::string room) 
{
	writeDataStringAttr("room", room);
	m_room = room;
}

void DataFile::setArray(std::string array)
{
	writeDataStringAttr("array", m_array);
	m_array = array;
}

void DataFile::setAnalogOutputSize(int size)
{
	writeDataAttr("analog-output-size", H5::PredType::STD_U64LE, &m_aoutSize);
	m_aoutSize = static_cast<decltype(m_aoutSize)>(size);
}

int DataFile::analogOutputSize() const
{
	return static_cast<int>(m_aoutSize);
}

arma::vec DataFile::analogOutput() const
{
	if (m_aoutSize == 0) {
		return arma::vec{};
	}
	auto sz = std::min(m_aoutSize, m_nsamples);
	return data(1, 0, sz);
}

void DataFile::readFileAttr(const std::string& name, void *buf) 
{
	try {
		H5::Attribute attr = m_file.openAttribute(name);
		attr.read(attr.getDataType(), buf);
	} catch (H5::FileIException &e) {
		std::cerr << "File exception accessing attribute: " << name << std::endl;
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	}
}

void DataFile::readDataAttr(const std::string& name, void *buf) 
{
	try {
		H5::Attribute attr = m_dataset.openAttribute(name);
		attr.read(attr.getDataType(), buf);
	} catch (H5::DataSetIException &e) {
		std::cerr << "DataSet exception accessing attribute: " << name << std::endl;
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	}
}

void DataFile::readDataStringAttr(const std::string& name, std::string &loc) 
{
	try {
		H5::Attribute attr = m_dataset.openAttribute(name);
		hsize_t sz = attr.getStorageSize();
		char *buf = new char[sz + 1]();
		if (buf == NULL)
			throw std::runtime_error("Error allocating memory for reading HDF string attribute");
		readDataAttr(name, buf);
		loc.clear();
		loc.resize(sz + 1);
		loc.replace(0, sz, buf);
		delete[] buf;
	} catch (H5::DataSetIException &e) {
		std::cerr << "DataSet exception accessing attribute: " << name << std::endl;
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	} catch (std::exception &e) {
		std::cerr << "Calloc error" << std::endl;
	}
}

void DataFile::readFileStringAttr(const std::string& name, std::string &loc) 
{
	try {
		H5::Attribute attr = m_file.openAttribute(name);
		hsize_t sz = attr.getStorageSize();
		char *buf = new char[sz + 1]();
		if (buf == NULL)
			throw std::runtime_error("Error allocating memory for reading HDF string attribute");
		readDataAttr(name, buf);
		loc.clear();
		loc.resize(sz + 1);
		loc.replace(0, sz, buf);
		delete[] buf;
	} catch (H5::DataSetIException &e) {
		std::cerr << "DataSet exception accessing attribute: " << name << std::endl;
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	} catch (std::exception &e) {
		std::cerr << "Calloc error" << std::endl;
	}
}

void DataFile::readSampleRate(void) 
{
	readDataAttr("sample-rate", &m_sampleRate);
}

void DataFile::readGain(void) 
{
	readDataAttr("gain", &m_gain);
}

void DataFile::readOffset(void) 
{
	readDataAttr("offset", &m_offset);
}

void DataFile::readNumSamples(void)
{
	if (m_dataset.attrExists("nsamples")) {
		readDataAttr("nsamples", &m_nsamples);
	} else {
		/* 
		 * Older versions of the library did not explicitly encode the
		 * number of samples. Fall back to the size of the dataset if
		 * needed.
		 */
		m_nsamples = datasetSize();
	}
}

void DataFile::readAnalogOutputSize(void)
{
	if (m_dataset.attrExists("analog-output-size")) {
		readDataAttr("analog-output-size", &m_aoutSize);
		/* 
		 * Older versions of the library did not explicitly encode
		 * whether analog output was performed in this recording.
		 * For simplicity, we just assume none was done in this
		 * case. The constructor sets this value to 0.
		 */
	}
}

void DataFile::readDate(void) 
{
	readDataStringAttr("date", m_date);
}

void DataFile::readRoom(void) 
{
	readDataStringAttr("room", m_room);
}

void DataFile::readArray()
{
	readDataStringAttr("array", m_array);
}

void DataFile::flush(void) 
{
	m_file.flush(H5F_SCOPE_GLOBAL);
}

std::string array(const std::string& fname)
{
	std::string a;
	try {
		H5::H5File f(fname, H5F_ACC_RDONLY);
		H5::DataSet dset = f.openDataSet("data");
		H5::Attribute attr = dset.openAttribute("array");
		hsize_t sz = attr.getStorageSize();
		a.resize(sz);
		attr.read(attr.getStrType(), a);
		attr.close();
		dset.close();
		f.close();
	} catch ( ... ) {
	}
	return a;
}

void DataFile::verifyWriteRequest(int startSample, int endSample)
{
	if (readOnly()) {
		throw std::logic_error("Cannot write to DataFile marked read-only.");
	}

	/* Validate requested samples */
	int requestedSamples = endSample - startSample;
	if (requestedSamples <= 0) {
		throw std::logic_error("Requested sample range invalid: (" + 
				std::to_string(startSample) + "-" + 
				std::to_string(endSample) + ")");
	}

	/* Extend dataset if needed */
	if (endSample > datasetSize()) {
		hsize_t dims[DatasetRank] = {0, 0};
		m_dataspace = m_dataset.getSpace();
		m_dataspace.getSimpleExtentDims(dims);
		auto nblocks = static_cast<int>(std::ceil(
				static_cast<float>(endSample - datasetSize()) /
				static_cast<float>(BlockSize)));
		dims[1] += nblocks * BlockSize;
		m_dataset.extend(dims);
		m_dataspace = m_dataset.getSpace();
	}
	m_nsamples = static_cast<uint64_t>(endSample);
}

H5::DataSpace DataFile::setupWrite(int startSample, int endSample)
{
	/* Compute the destination file data space */
	int requestedSamples = endSample - startSample;
		hsize_t memoffset[DatasetRank] = {0,
			static_cast<hsize_t>(startSample)};
	hsize_t memcount[DatasetRank] = {
			static_cast<hsize_t>(nchannels()),
			static_cast<hsize_t>(requestedSamples)};
	m_dataspace.selectHyperslab(H5S_SELECT_SET, memcount, memoffset);
	if (!m_dataspace.selectValid()) {
		std::stringstream what;
		what << "Dataset selection invalid:" << std::endl
				<< "Offset: (" << startSample << ", 0)" << std::endl
				<< "Count: (" << requestedSamples << ", "
				<< nchannels() << ")" << std::endl;
		throw std::logic_error(what.str());
	}

	/* Define the source data space in memory */
	hsize_t dims[DatasetRank] = {
			static_cast<hsize_t>(nchannels()),
			static_cast<hsize_t>(requestedSamples)};
	H5::DataSpace memspace = {DatasetRank, dims};
	hsize_t offset[DatasetRank] = {0, 0};
	hsize_t count[DatasetRank] = {
			static_cast<hsize_t>(nchannels()),
			static_cast<hsize_t>(requestedSamples)};
	memspace.selectHyperslab(H5S_SELECT_SET, count, offset);
	if (!memspace.selectValid()) {
		std::stringstream what;
		what << "Memory dataspace selection invalid:" << std::endl
				<< "Count: (" << requestedSamples << ", "
				<< nchannels() << ")" << std::endl;
		throw std::logic_error(what.str());
	}
	return memspace;
}

int DataFile::datasetSize() const {
	hsize_t dims[DatasetRank] = { 0, 0 };
	m_dataspace.getSimpleExtentDims(dims);
	return static_cast<int>(dims[1]);
}

void DataFile::setMeans(const arma::vec& means)
{
	const char name[] = "channel-means";
	if (m_dataset.attrExists(name)) {
		m_dataset.removeAttr(name);
	}
	hsize_t dims[1] = { static_cast<hsize_t>(means.n_elem) };
	auto space = H5::DataSpace(1, dims);
	auto attr = m_dataset.createAttribute(name, H5::PredType::IEEE_F64LE, space);
	attr.write(H5::PredType::IEEE_F64LE, means.memptr());
	attr.close();
}

arma::vec DataFile::means() const
{
	arma::vec ret;
	H5::Attribute attr;
	try {
		attr = m_dataset.openAttribute("channel-means");
	} catch (H5::AttributeIException& e) {
		return ret;
	}

	auto space = attr.getSpace();
	hsize_t dims[1] = { 0 };
	space.getSimpleExtentDims(dims);
	ret.set_size(dims[0]);
	attr.read(H5::PredType::IEEE_F64LE, ret.memptr());
	attr.close();
	return ret;
}

} // end datafile namespace

