/*! \file datafile.h
 *
 * Public API for the DataFile class, which represents an HDF5 data
 * file to which experimental data is saved.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _DATAFILE_H_
#define _DATAFILE_H_

#include "H5Cpp.h"
#include <armadillo>

#include <string>
#include <vector>

/*! The datafile namespace contains classes and constants related
 * to the file format for storing data from Baccus lab experiments.
 */
namespace datafile {

/*! File extension for all recording files created by libdatafile */
const std::string FileExtension = ".h5";

/*! Rank of all datasets */
const int DatasetRank = 2;

/*! Default number of channels.
 * Note that this is the number of channels for an MCS array recording, 
 * not necessarily a HiDens recording. However it is defined here so that
 * both the base DataFile class and the HidensFile subclass can use the
 * same constructor
 */
const int NumChannels = 64;

/*! Maximum number of channels in any recording */
const int MaxNumChannels = 128;

/*! Chunk size for HDF5 library reads and writes. */
const int BlockSize = 20000;

/*! Default sample rate for MCS array data */
const float SampleRate = 10000;

/*! Number of chunks to request the HDF5 library keep cached. */
const unsigned int ChunkCacheSize = 5;

/*! Default dimensions of the dataset */
const hsize_t DatasetDefaultDims[DatasetRank] = { NumChannels, BlockSize };

/*! Chunk size for the data set in each dimension. */
const hsize_t DatasetChunkDims[DatasetRank] = { NumChannels, BlockSize };

/*! Maximum dimensions for the dataset. */
const hsize_t DatasetMaxDims[DatasetRank] = { MaxNumChannels, H5S_UNLIMITED };

/*! String stored in the file giving the room in which data was recorded. */
const std::string DefaultRoomString = "recorded in d239";

/*! Format in which to save the date of a recording */
const char DateFormat[] = "%Y-%m-%dT%H:%M:%S";

/*! Default array to use when creating a new recording */
const std::string DefaultArray = "mcs";

/*! Type aliases for data from arrays */
using samples = arma::mat; 				// true voltage units
using ssamples = arma::Mat<int16_t>;	// data from MCS arrays
using usamples = arma::Mat<uint8_t>;	// data from HiDens arrays

/* Template methods for determining H5 datatype from the datatype of
 * and Armadillo matrix or vector. These are used to enable correct 
 * conversion of data to/from the file and in-memory matrices of
 * various data types. These allow compile-time creation of the 
 * correct HDF5 datatype, rather than runtime selection. Add more
 * overloads returning the correct HDF5 datatype if needed.
 */
template<class T>
H5::DataType dtypeForMat(const arma::Mat<T>& /* mat */, 
		typename std::enable_if<std::is_same<T, double>::value>::type* = nullptr)
{
	return H5::PredType::IEEE_F64LE;
}

template<class T>
H5::DataType dtypeForMat(const arma::Mat<T>& /* mat */, 
		typename std::enable_if<std::is_same<T, float>::value>::type* = nullptr)
{
	return H5::PredType::IEEE_F32LE;
}

template<class T>
H5::DataType dtypeForMat(const arma::Mat<T>& /* mat */,
		typename std::enable_if<std::is_same<T, int16_t>::value>::type* = nullptr)
{
	return H5::PredType::STD_I16LE;
}

template<class T>
H5::DataType dtypeForMat(const arma::Mat<T>& /* mat */,
		typename std::enable_if<std::is_same<T, uint8_t>::value>::type* = nullptr)
{
	return H5::PredType::STD_U8LE;
}

template<class T>
H5::DataType dtypeForMat(const arma::Mat<T>& /* mat */,
		typename std::enable_if<std::is_same<T, int>::value>::type* = nullptr)
{
	return H5::PredType::NATIVE_INT;
}

template<class T>
H5::DataType dtypeForMat(const arma::Mat<T>& /* mat */,
		typename std::enable_if<std::is_same<T, uint32_t>::value>::type* = nullptr)
{
	return H5::PredType::STD_U32LE;
}

template<class T>
H5::DataType dtypeForMat(const arma::Mat<T>& /* mat */,
		typename std::enable_if<std::is_same<T, uint16_t>::value>::type* = nullptr)
{
	return H5::PredType::STD_U16LE;
}

/*! The DataFile class is the heart of libdatafile. It provides functionality
 * for reading, writing, and modifying an HDF5 recording file in the Baccus Lab.
 */
class DataFile {

	public:

		/*! Static public method used to read array type from the given data file. */
		static std::string array(const std::string& filename);

		/*! Construct a new DataFile object. 
		 * The file is created if it does not exist, otherwise it is opened read-only.
		 * \param filename The name of the file to create or open.
		 * \param array The type of array the written data will come from.
		 * \param nchannels The number of channels to be written to the dataset.
		 */
		DataFile(const std::string& filename, 
				const std::string& array = DefaultArray,
				const hsize_t nchannels = NumChannels);

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

		/*! Return the size of any analog output used in this recording. */
		int analogOutputSize() const;

		/*! Return the analog output used in this recording.
		 * If there was no analog output, the returned vector will be empty.
		 *
		 * This is defined as a virtual function so that, if it is implemented
		 * in the future, HiDens files may define which channels contain
		 * the analog output.
		 */
		virtual arma::vec analogOutput() const;

		/*! Set the size of any analog output used in the recording.
		 * \param sz The size in samples of the output.
		 *
		 * Note that this is just used for book-keeping purposes to 
		 * know whether or not any analog output was performed during
		 * the recording this file represents. The actual values for
		 * the analog output are stored in the file itself (e.g.,
		 * as channel 1 for MCS data files.). The vector of values
		 * can be returned using DataFile::analogOutput().
		 *
		 * This is defined as a virtual function because Hidens
		 * files do not currently support analog output, but that
		 * may change in the future.
		 */
		virtual void setAnalogOutputSize(int sz);

		/*! Return data from all channels over the given sample rate.
		 * Data is return in true voltage units, as double-precision IEEE floats.
		 *
		 * \param start The first sample to return
		 * \param end The last sample to return.
		 * 
		 * NOTE: Data is returned in an Armadillo matrix with size (nsamples, nchannels).
		 * Because Armadillo uses column-order majoring, this corresponds to the
		 * HDF5 dataset with size (nchannels, nsamples).
		 *
		 * Exceptions:
		 * This will throw a std::logic_error if either the requested channels
		 * or samples are outside of the range for the file.
		 */
		samples data(int start, int end) const;

		/*! Return data from the given channel.
		 * Data is returned in true voltage units of the ADC.
		 *
		 * \param channel The channel whose data should be returned
		 * \param start The first sample to return.
		 * \param end The last sample to return.
		 *
		 * Exceptions:
		 * This will throw a std::logic_error if either the requested channels
		 * or samples are outside of the range for the file.
		 */
		arma::vec data(int channel, int start, int end) const;

		/* Read data from a contiguous set of channels into the given matrix.
		 * \param startChan The first channel to read
		 * \param endChan The last channel to read
		 * \param startSample The first sample to read.
		 * \param lastSample The last sample to read.
		 * \param data The Armadillo matrix to fill with the requested data. Data
		 * will be converted to the appropriate type to fill the given matrix.
		 * 
		 * NOTE: Data is returned in an Armadillo matrix with size (nsamples, nchannels).
		 * Because Armadillo uses column-order majoring, this corresponds to the
		 * HDF5 dataset with size (nchannels, nsamples).
		 *
		 * Exceptions:
		 * This will throw a std::logic_error if either the requested channels
		 * or samples are outside of the range for the file.
		 */
		template<class T>
		void data(int startChan, int endChan, 
				int startSample, int endSample, arma::Mat<T>& data) const
		{
			verifyReadRequest(startChan, endChan, startSample, endSample);
			auto memspace = setupRead(startChan, endChan, startSample, endSample);
			data.set_size(endSample - startSample, endChan - startChan);
			m_dataset.read(data.memptr(), dtypeForMat(data), memspace, m_dataspace);
		}

		/* Write data to the file.
		 * \param startSample The first sample to write.
		 * \param endSample The last sample to write.
		 * \param data The Armadillo matrix containing data to write.
		 * 
		 * NOTE: Data should be in an Armadillo matrix with size (nsamples, nchannels).
		 * Because Armadillo uses column-order majoring, this corresponds to the
		 * HDF5 dataset with size (nchannels, nsamples).
		 *
		 * Exceptions:
		 * This will throw a std::logic_error endSample <= startSample. If endSample
		 * is beyond the current end of the dataset, *it will be extended* to the next
		 * largest multiple of the BLOCK_SIZE required to accommodate the data.
		 */
		template<class T>
		void setData(int startSample, int endSample, const arma::Mat<T>& data) { 
			verifyWriteRequest(startSample, endSample);
			auto memtype = dtypeForMat(data);
			auto memspace = setupWrite(startSample, endSample);
			m_dataset.write(data.memptr(), memtype, memspace, m_dataspace);
			flush();
		}

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
		const H5::DataType& dtype() const { return m_datatype; }

		/*! Write a dataset containing the mean value of each channel's data.
		 * This is computed and saved while running the `extract` program, which
		 * extracts candidate spike snippets, and used during spike sorting.
		 */
		void setMeans(const arma::vec& means);

		/*! Read the mean values stored for each channel.
		 * An empty vector is returned if the mean values have not yet been computed.
		 */
		arma::vec means() const;

	protected:
		void flush();			// Flush the file to disk

		/* Read the available size of the dataset, in samples */
		int datasetSize() const;

		/* Read or write underlying dataset or file HDF5 attributes */
		void writeDataAttr(const std::string& name, const H5::DataType &type, void *buf);
		void writeDataStringAttr(const std::string& name, const std::string& value);
		void writeAllAttributes();
		void readFileAttr(const std::string& name, void *buf);
		void readDataAttr(const std::string& name, void *buf);
		void readDataStringAttr(const std::string& name, std::string &dst);
		void readFileStringAttr(const std::string& name, std::string &dst);

		/* Read the corresponding values from the file */
		void readArray();
		void readSampleRate();
		void readGain();
		void readOffset();
		void readDate();
		void readRoom();
		void readNumSamples();
		void readAnalogOutputSize();

		H5::H5File m_file;				// The actual HDF5 file
		H5::DataSpace m_dataspace;		// Data space for actual data
		H5::DataType m_datatype;		// Type for the actual data
		H5::DSetCreatPropList m_props;	// Properties for the dataset (chunking, etc)
		H5::DataSet m_dataset;			// The HDF5 dataset containing data
		bool m_readOnly;				// Protection

		std::string m_filename;		// Full path name of HDF5 file
		std::string m_array;		// Array type
		float m_sampleRate;			// Data sample rate
		float m_gain;				// Gain of A/D conversion
		float m_offset;				// Offset of A/D conversion
		std::string m_date;			// Date of recording, ISO-8601 format
		std::string m_room; 		// Location of recording
		uint64_t m_nsamples;		// Total number of samples written
		uint64_t m_aoutSize;		// Size of any analog output used in the recording

		bool readOnly() const { return m_readOnly; }

		/* Throw a std::logic_error if the requested write parameters are invalid.
		 * This resizes the file's dataset if needed.
		 */
		void verifyWriteRequest(int startSample, int endSample);

		/* Create a memory (source) dataspace and set up the file (dest)
		 * dataspace for a write of data. This takes care of a lot of 
		 * H5 library boilerplate that is shared across the different
		 * setData() overloads.
		 */
		H5::DataSpace setupWrite(int startSample, int endSample);

		/* Throw a std::logic_error if the requested read parameters are invalid. */
		void verifyReadRequest(int startChannel, int endChannel, 
				int startSample, int endSample) const;

		/* Create a memory (destination) dataspace and setup the file (source)
		 * dataspace for a read of data.
		 */
		H5::DataSpace setupRead(int startChannel, int endChannel, 
				int startSample, int endSample) const;



}; // End class


}; // end datafile namespace

#endif

