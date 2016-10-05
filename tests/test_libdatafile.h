/*! \file test_libdatafile.h
 *
 * Class definitions for testing libdatafile code.
 *
 * (C) 2016 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef TEST_LIBDATAFILE_H_
#define TEST_LIBDATAFILE_H_

#include "../include/datafile.h"
#include "../include/hidensfile.h"
#include "../include/snipfile.h"
#include "../include/hidenssnipfile.h"

#include <QtCore>
#include <QtTest/QtTest>

using namespace datafile;
using namespace hidensfile;
using namespace snipfile;
using namespace hidenssnipfile;

class DatafileTest : public QObject {
	Q_OBJECT

	private slots:

		/*! Initialization performed before all tests. */
		void initTestCase();

		/*! Cleanup perfomed after all test cases. */
		void cleanupTestCase();

		/*! Try opening an existing files that are not in the correct
		 * format, and verify that exceptions are thrown.
		 */
		void testBadFile();

		/*! Test writing and reading expected attributes of the data file. */
		void testBasicAttributes();

		/*! Test that writing a small chunk of data to the file
		 * and reading it back works.
		 */
		void testSmallDataReadWrite();

		/*! Verify that reading past the end of the data file throws. */
		void testReadPastEnd();

		/*! Verify that writing past the end of the file extends it */
		void testExtendFile();

		/*! Attempt to write to a pre-existing data file, and verify that
		 * an exception is thrown.
		 */
		void testWriteToExistingFile();

		/*! Test reading and writing a configuration to a Hidens file */
		void testConfiguration();

		/*! Test creating basic snippet files from data files */
		void testCreateSnippetFiles();

		/*! Test that the source information in the data file is copied
		 * to the corresponding snippet files.
		 */
		void testSourceInformationCopied();

		/*! Test reading/writing the extracted channels to the snippet files */
		void testReadWriteChannels();

		/*! Test that reading/writing the thresholds for extracted channels works. */
		void testReadWriteThresholds();

		/*! Test adding random snippets to the files and reading them back */
		void testReadWriteSnippets();

		/*! Test reading writing means to the original raw data files. 
		 * This process is done by the `extract` program, to save the mean
		 * value of each channel.
		 */
		void testReadWriteMeans();

	private:
		QString m_datafileName;
		QString m_hidensfileName;
		QString m_snipfileName;
		QString m_hidensSnipfileName;

		std::unique_ptr<DataFile> m_dataFile;
		std::unique_ptr<HidensFile> m_hidensFile;
		std::unique_ptr<SnipFile> m_snipFile;
		std::unique_ptr<HidensSnipFile> m_hidensSnipfile;

		arma::Mat<qint16> m_data;
		arma::Mat<qint16> m_hidensData;

		Configuration m_config;
};


#endif
