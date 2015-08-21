/* datafile.h
 * Public API for the DataFile class, which represents an HDF5 data
 * file to which experimental data is saved
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef MEAREC_DATAFILE_H_
#define MEAREC_DATAFILE_H_

/* C++ standard library */
#include <string>
#include <vector>

/* Third-party includes */
#include "H5Cpp.h"
#include <armadillo>

namespace datafile {

/* Constants */
const std::string RECORDING_FILE_EXTENSION = ".h5";
const int DATASET_RANK = 2;
const int NUM_CHANNELS = 64;
const int BLOCK_SIZE = 20000;
const float SAMPLE_RATE = 10000;
const unsigned int CHUNK_CACHE_SIZE = 5; // In number of chunks
const hsize_t DATASET_DEFAULT_DIMS[DATASET_RANK] = {NUM_CHANNELS, BLOCK_SIZE};
const hsize_t DATASET_CHUNK_DIMS[DATASET_RANK]= {NUM_CHANNELS, BLOCK_SIZE};
const hsize_t DATASET_MAX_DIMS[DATASET_RANK] = {NUM_CHANNELS, H5S_UNLIMITED};
const std::string DEFAULT_ROOM_STRING("recorded in d239");
const char DATE_FORMAT[] = "%Y-%m-%dT%H:%M:%S";
const std::string DEFAULT_ARRAY("hexagonal");

/* Aliases for Armadillo matrices used to store data */
using samples = arma::mat;				// Used for either MCS or HiDens 
using ssamples = arma::Mat<short>;		// Used for MCS recordings
using usamples = arma::Mat<uint8_t>; 	// Used for HiDens recordings

class DataFile {

	public:
		/* Construct a new DataFile object. The file is created if it does
		 * not exist, otherwise it is opened read-only.
		 */
		DataFile(const std::string& filename, 
				const std::string& array = DEFAULT_ARRAY,
				const hsize_t nchannels = NUM_CHANNELS);
		virtual ~DataFile();

		std::string filename() const;	// Returns the full pathname of the file
		std::string array() const;		// Array on which data was recorded
		double length() const;			// Returns the recording's length in seconds
		uint32_t nsamples() const;		// Returns the number of samples
		uint32_t nchannels() const;		// Returns the number of channels
		float sampleRate() const;		// Returns the data sample rate
		float gain() const;				// Returns the NI-DAQ ADC gain
		float offset() const;			// Returns the NI-DAQ ADC offset
		std::string date() const;		// Returns date of the recording
		std::string room() const;		// Returns room in which recording occurred

		/* Return data in double-precision, true voltage values */
		samples data(int start, int end) const;
		arma::vec data(int channel, int start, int end) const;

		/* Return data from single channel of given type */
		template<class T>
		void data(int channel, int start, int end, T& data) const;

		/* Return data from contiguous block of channels */
		template<class T>
		void data(int startChan, int endChan, 
				int startSample, int endSample, T& data) const;

		/* Return data from arbitrary selection of channels */
		template<class T>
		void data(const arma::uvec& channels, int start, int end, T& data) const;

		/* Write data to the file. This method will extend the 
		 * dataset, if needed.
		 * See datafile-templates.cc for implementation
		 */
		template<class T>
		void setData(int startSample, int endSample, const T& data);

		/* Setters for data attributes */
		void setFilename(std::string filename);
		void setArray(std::string array);
		void setSampleRate(float sampleRate);
		void setGain(float gain);
		void setOffset(float offset);
		void setDate(void);
		void setRoom(std::string room);

		const H5::DataType& dtype() const { return datatype; }

	protected:
		void flush();			// Flush the file to disk

		/* Read or write underlying dataset or file HDF5 attributes */
		void writeFileAttr(std::string name, const H5::DataType &type, void *buf);
		void writeDataAttr(std::string name, const H5::DataType &type, void *buf);
		void writeDataStringAttr(std::string name, std::string value);
		void writeFileStringAttr(std::string name, std::string value);
		void writeAllAttributes();
		void readFileAttr(std::string name, void *buf);
		void readDataAttr(std::string name, void *buf);
		void readDataStringAttr(std::string name, std::string &dst);
		void readFileStringAttr(std::string name, std::string &dst);

		void computeCoords(const arma::uvec& channels, 
				int start, int end, arma::Mat<hsize_t>& out, hsize_t *nelem) const;

		/* Read the corresponding values from the file */
		void readArray();
		void readSampleRate();
		void readGain();
		void readOffset();
		void readDate();
		void readRoom();

		H5::H5File file;				// The actual HDF5 file
		H5::DataSpace dataspace;		// Data space for actual data
		H5::DataType datatype;			// Type for the actual data
		H5::DSetCreatPropList props;	// Properties for the dataset (chunking, etc)
		H5::DataSet dataset;			// The HDF5 dataset containing data
		bool readOnly;					// Protection

		std::string filename_;		// Full path name of HDF5 file
		std::string array_;			// Array type
		uint32_t nchannels_;		// Number of channels
		float sampleRate_;			// Data sample rate
		float gain_;				// Gain of A/D conversion
		float offset_;				// Offset of A/D conversion
		std::string date_;			// Date of recording, ISO-8601 format
		std::string room_; 			// Location of recording

}; // End class
}; // End namespace

#include "datafile-templates.cc" // Implementation of templated functions defined above

#endif

