/*! \file datafile.h
 *
 * Public API for the DataFile class, which represents an HDF5 data
 * file to which experimental data is saved
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _DATAFILE_H_
#define _DATAFILE_H_

#include "H5Cpp.h"
#include <armadillo>

#include <string>
#include <vector>

/*! Namespace containing all data and code relating to libdatafile. */
namespace datafile {

/*! File extension for all recording files created by libdatafile */
const std::string RECORDING_FILE_EXTENSION = ".h5";

/*! Rank of all datasets */
const int DATASET_RANK = 2;

/*! Default number of channels.
 * Note that this is the number of channels for an MCS array recording, 
 * not necessarily a HiDens recording. However it is defined here so that
 * both the base DataFile class and the HidensFile subclass can use the
 * same constructor
 */
const int NUM_CHANNELS = 64;

/*! Maximum number of channels in any recording */
const int MAX_NUM_CHANNELS = 128;

/*! Chunk size for HDF5 library reads and writes. */
const int BLOCK_SIZE = 20000;

/*! Default sample rate for MCS array data */
const float SAMPLE_RATE = 10000;

/*! Number of chunks to request the HDF5 library keep cached */
const unsigned int CHUNK_CACHE_SIZE = 5; // In number of chunks

/*! Default dimensions of the dataset */
const hsize_t DATASET_DEFAULT_DIMS[DATASET_RANK] = {NUM_CHANNELS, BLOCK_SIZE};

/*! Chunksize for the dataset along each dimension */
const hsize_t DATASET_CHUNK_DIMS[DATASET_RANK]= {NUM_CHANNELS, BLOCK_SIZE};

/*! Maximum dimensions for the dataset, so that it can be
 * extended as a recording grows
 */
const hsize_t DATASET_MAX_DIMS[DATASET_RANK] = {MAX_NUM_CHANNELS, H5S_UNLIMITED};

/*! String stored in the file giving the room in which data was recorded. */
const std::string DEFAULT_ROOM_STRING = "recorded in d239";

/*! Format in which to save the date of the recording */
const char DATE_FORMAT[] = "%Y-%m-%dT%H:%M:%S";

/*! Default array to use when creating a new recording */
const std::string DEFAULT_ARRAY = "hexagonal";

/*! Type alias for data from either array */
using samples = arma::mat;

/*! Type alias for data from the MCS arrays only */
using ssamples = arma::Mat<short>;

/*! Type alias for adata from the HiDens arrays only */
using usamples = arma::Mat<uint8_t>;

std::string array(const std::string& filename);

/*! The DataFile class is the heart of libdatafile. It provides functionality
 * for reading, writing, and modifying an HDF5 recording file in the Baccus Lab.
 */
class DataFile {

	public:
		/*! Construct a new DataFile object. 
		 * The file is created if it does not exist, otherwise it is opened read-only.
		 * \param filename The name of the file to create or open.
		 * \param array The type of array the written data will come from.
		 * \param nchannels The number of channels to be written to the dataset.
		 */
		DataFile(const std::string& filename, 
				const std::string& array = DEFAULT_ARRAY,
				const hsize_t nchannels = NUM_CHANNELS);

		/*! Destroy a DataFile, flushing and closing the underlying file */
		virtual ~DataFile();

		/*! Return the full pathname of the file */
		std::string filename() const;
		
		/*! Return the array from which the data was recorded */
		std::string array() const;

		/*! Return the length of the full recording, in seconds */
		double length() const;

		/*! Return the total number of samples in the recording */
		int nsamples() const;

		/*! Return the number of channels in the data file */
		int nchannels() const;

		/*! Return the sample rate of the data */
		float sampleRate() const;

		/*! Return the total gain of the the analog-digital conversion stage
		 * when recording the data.
		 *
		 * Note that this means different things for MCS and HiDens arrays.
		 * For example, the NI-DAQ used to record data from the MCS arrays
		 * converts into units of volts, while the HiDens system converts 
		 * to microvolts.
		 */
		float gain() const;

		/*! Return the voltage offset of the analog-digital conversion stage
		 * when recording the data.
		 */
		float offset() const;

		/*! Return the date on which the data was recorded */
		std::string date() const;

		/*! Return the room in which the data was recorded */
		std::string room() const;

		/*! Return data from all channels over the given sample rate.
		 * Data is return in true voltage units, as double-precision IEEE floats.
		 *
		 * \param start The first sample to return
		 * \param end The last sample to return.
		 */
		samples data(int start, int end) const;

		/*! Return data from the given channel.
		 * \param channel The channel whose data should be returned
		 * \param start The first sample to return.
		 * \param end The last sample to return.
		 */
		arma::vec data(int channel, int start, int end) const;

		/* Fill the given Armadillo vector with data from the requested channel.
		 * \param channel The channel to read data from
		 * \param start The first sample to read
		 * \param end The last sample to read
		 * \param data The Armadillo matrix to fill with the requested data. Data
		 * will be converted to the appropriate type to fill the given matrix.
		 */
		template<class T>
		void data(int channel, int start, int end, T& data) const;

		/* Read data from a contiguous set of channels into the given matrix.
		 * \param startChan The first channel to read
		 * \param endChan The last channel to read
		 * \param startSample The first sample to read.
		 * \param lastSample The last sample to read.
		 * \param data The Armadillo matrix to fill with the requested data. Data
		 * will be converted to the appropriate type to fill the given matrix.
		 */
		template<class T>
		void data(int startChan, int endChan, 
				int startSample, int endSample, arma::Mat<T>& data) const;

		/* Read data from an arbitrary selection of channels into the given matrix.
		 * This form of the function reads data from the channels whose indices
		 * are given in the first parameter. The channels need not be contiguous.
		 * Note that, if a contiguous selection of channels is desired, it can
		 * be hugely more effecient to use the other version of this function.
		 * \param channels A vector of channel indices to read.
		 * \param start The first sample to read.
		 * \param end The last sample to read.
		 * \param data The Armadillo matrix to fill with the requested data. Data
		 * will be converted to the appropriate type to fill the given matrix.
		 */
		template<class T>
		void data(const arma::uvec& channels, int start, int end, T& data) const;

		/* Write data to the file.
		 * \param startSample The first sample to write.
		 * \param endSample The last sample to write.
		 * \param data The Armadillo matrix containing data to write.
		 */
		void setData(int startSample, int endSample, const arma::Mat<uint8_t>& data);

		/* Write data to the file.
		 * \param startSample The first sample to write.
		 * \param endSample The last sample to write.
		 * \param data The Armadillo matrix containing data to write.
		 */
		void setData(int startSample, int endSample, const arma::Mat<int16_t>& data);

		/* Write data to the file.
		 * \param startSample The first sample to write.
		 * \param endSample The last sample to write.
		 * \param data The Armadillo matrix containing data to write.
		 */
		void setData(int startSample, int endSample, const arma::Mat<double>& data);

		/*! Set the array from which data in this file derives.
		 * \param array The array type.
		 */
		void setArray(std::string array);

		/*! Set the sample rate of the data */
		void setSampleRate(float sampleRate);

		/*! Set the ADC gain of the data. */
		void setGain(float gain);

		/*! Set the ADC offset for the data */
		void setOffset(float offset);

		/*! Set the date */
		void setDate(std::string date);

		/*! Set the room string */
		void setRoom(std::string room);

		/*! Returns the HDF5 dataype for the underlying dataset. */
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
		void readNumSamples();

		H5::H5File file;				// The actual HDF5 file
		H5::DataSpace dataspace;		// Data space for actual data
		H5::DataType datatype;			// Type for the actual data
		H5::DSetCreatPropList props;	// Properties for the dataset (chunking, etc)
		H5::DataSet dataset;			// The HDF5 dataset containing data
		bool readOnly;					// Protection

		std::string filename_;		// Full path name of HDF5 file
		std::string array_;			// Array type
		float sampleRate_;			// Data sample rate
		float gain_;				// Gain of A/D conversion
		float offset_;				// Offset of A/D conversion
		std::string date_;			// Date of recording, ISO-8601 format
		std::string room_; 			// Location of recording
		uint64_t nsamples_;			// Total number of samples written

		/* Create a memory (source) dataspace and set up the file (dest)
		 * dataspace for a write of data. This takes care of a lot of 
		 * H5 library boilerplate that is shared across the different
		 * setData() overloads.
		 */
		H5::DataSpace setupWrite(int startSample, int endSample);

}; // End class
}; // End namespace

#include "datafile-templates.cc" // Implementation of templated functions defined above

#endif

