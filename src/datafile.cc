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
		: filename_(filename),
		  array_(array)
{
	/* If file exists, verify it is valid HDF5 and load data from it.
	 * Else, construct a new file.
	 */
	struct stat buffer;
	if (stat(filename_.c_str(), &buffer) == 0) {
		if (!H5::H5File::isHdf5(filename_)) {
			std::cerr << "Invalid H5 file" << std::endl;
			throw std::invalid_argument("Invalid HDF5 file");
		}
		try {
			readOnly = true;
			file = H5::H5File(filename_, H5F_ACC_RDONLY);
		} catch (H5::FileIException &e) {
			std::cerr << "Could not open H5 file" << std::endl;
			throw std::runtime_error("Could not open HDF5 file");
		}

		/* Open the dataset */
		try {
			dataset = file.openDataSet("data");
		} catch (H5::FileIException &e) {
			std::cerr << "File must contain a dataset labeled 'data'";
			throw std::invalid_argument("File must contain a 'data' dataset");
		}
		dataspace = dataset.getSpace();
		datatype = dataset.getDataType();

		/* Read attributes into data members */
		try {
			readArray();
			readSampleRate();
			readGain();
			readOffset();
			readDate();
			readRoom();
		} catch ( ... ) {
			std::cerr << "File does not contain appropriate attributes";
			throw std::invalid_argument(
					"H5 file does not contain appropriate attributes");
		}

	} else {
		/* Construct the file. Define to have a chunk cache large enough to hold
		 * a few chunks at a time.
		 */
		readOnly = false;
		H5::FileAccPropList fileProps(H5::FileAccPropList::DEFAULT);
		int mdc_nelmts = 0;
		size_t rdcc_nelmts, rdcc_nbytes = 0;
		size_t chunkCacheSizeElems = (
				datafile::CHUNK_CACHE_SIZE * datafile::BLOCK_SIZE * 
				datafile::NUM_CHANNELS
			);
		double rdcc_w0 = 0.0;
		fileProps.getCache(mdc_nelmts, rdcc_nelmts, rdcc_nbytes, rdcc_w0);
		fileProps.setCache(mdc_nelmts, chunkCacheSizeElems, 
				chunkCacheSizeElems * sizeof(int16_t), rdcc_w0);
		file = H5::H5File(filename_, H5F_ACC_TRUNC, 
				H5::FileCreatPropList::DEFAULT, fileProps);

		/* Create the dataset */
		hsize_t dims[DATASET_RANK] = {nchannels, DATASET_DEFAULT_DIMS[1]};
		dataspace = H5::DataSpace(datafile::DATASET_RANK, dims,
				datafile::DATASET_MAX_DIMS);
		props = H5::DSetCreatPropList();
		props.setChunk(datafile::DATASET_RANK, datafile::DATASET_CHUNK_DIMS);
		datatype = H5::DataType(H5::PredType::STD_I16LE);
		dataset = file.createDataSet("data", datatype, dataspace, props);

		/* Set default parameters */
		setSampleRate(datafile::SAMPLE_RATE);
		setRoom(datafile::DEFAULT_ROOM_STRING);
	}
}

DataFile::~DataFile() 
{
	try {
		if (!readOnly) 
			writeAllAttributes();
		file.close();
	} catch (H5::FileIException &e) {
		std::cerr << "Error closing HDF5 file: " << filename_ << std::endl;
	}
}

std::string DataFile::filename() const { return filename_; }

std::string DataFile::array() const { return array_; }

double DataFile::length() const 
{ 
	return ((double) nsamples() / sampleRate());
}

int DataFile::nsamples() const
{
	hsize_t dims[DATASET_RANK] = {0, 0};
	dataspace.getSimpleExtentDims(dims);
	return dims[1];
}

int DataFile::nchannels() const
{
	hsize_t dims[DATASET_RANK] = {0, 0};
	dataspace.getSimpleExtentDims(dims);
	return dims[0];
}

float DataFile::sampleRate(void) const { return sampleRate_; }

float DataFile::gain(void) const { return gain_; }

float DataFile::offset(void) const { return offset_; }

std::string DataFile::date(void) const { return date_; }

std::string DataFile::room(void) const { return room_; }

samples DataFile::data(int startSample, int endSample) const
{
	/* Allocate return array */
	int req_nsamples = endSample - startSample;
	if (req_nsamples < 0) {
		std::cerr << "Requested sample range is invalid: (" << 
				startSample << ", " << endSample << ")" << std::endl;
		throw std::logic_error("Requested sample range invalid");
	}
	samples s(req_nsamples, nchannels());
	data(0, nchannels(), startSample, endSample, s);
	return s;
}

arma::vec DataFile::data(int channel, int startSample, int endSample) const
{
	int req_nsamples = endSample - startSample;
	if (req_nsamples < 0) {
		std::cerr << "Requested sample range is invalid: (" << 
				startSample << ", " << endSample << ")" << std::endl;
		throw std::logic_error("Requested sample range invalid");
	}
	if ( (channel < 0) || (channel >= nchannels()) ) {
		std::cerr << "Requested channel is invalid: " << channel << std::endl;
		throw std::logic_error("Requested channel invalid");
	}
	arma::vec s(req_nsamples);
	data(channel, startSample, endSample, s);
	return s;
}

void DataFile::writeFileAttr(std::string name, const H5::DataType &type, void *buf) 
{
	if (readOnly)
		return;
	try {
		H5::DataType writeType(type);
		if (!(file.attrExists(name))) {
			H5::DataSpace space(H5S_SCALAR);
			file.createAttribute(name, type, space);
		}
		H5::Attribute attr = file.openAttribute(name);
		attr.write(writeType, buf);
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	} catch (H5::FileIException &e) {
		std::cerr << "File exception accessing: " << name << std::endl;
	}
}

void DataFile::writeDataAttr(std::string name, const H5::DataType &type, void *buf) 
{
	if (readOnly)
		return;
	try {
		H5::DataType writeType(type);
		if (!(dataset.attrExists(name))) {
			H5::DataSpace space(H5S_SCALAR);
			dataset.createAttribute(name, writeType, space);
		}
		H5::Attribute attr = dataset.openAttribute(name);
		attr.write(writeType, buf);
	} catch (H5::DataSetIException &e) {
		std::cerr << "DataSet exception accessing: " << name << std::endl;
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	}
}

void DataFile::writeDataStringAttr(std::string name, std::string value) 
{
	if (readOnly)
		return;
	try {
		H5::StrType stringType(0, value.length());
		if (!(dataset.attrExists(name))) {
			H5::DataSpace space(H5S_SCALAR);
			dataset.createAttribute(name, stringType, space);
		}
		H5::Attribute attr = dataset.openAttribute(name);
		attr.write(stringType, value.c_str());
	} catch (H5::DataSetIException &e) {
		std::cerr << "DataSet exception accessing: " << name << std::endl;
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	}
}

void DataFile::writeFileStringAttr(std::string name, std::string value)
{
	if (readOnly)
		return;
	try {
		H5::StrType stringType(0, value.length());
		if (!file.attrExists(name)) {
			H5::DataSpace space(H5S_SCALAR);
			file.createAttribute(name, stringType, space);
		}
		H5::Attribute attr = file.openAttribute(name);
		attr.write(stringType, value.c_str());
	} catch (H5::FileIException &e) {
		std::cerr << "File exception accessing: " << name << std::endl;
	} catch( H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	}
}

void DataFile::writeAllAttributes(void) 
{
	writeDataAttr("sample-rate", H5::PredType::IEEE_F32LE, &sampleRate_);
	writeDataAttr("gain", H5::PredType::IEEE_F32LE, &gain_);
	writeDataAttr("offset", H5::PredType::IEEE_F32LE, &offset_);
	writeDataStringAttr("array", array_);
	writeDataStringAttr("date", date_);
	writeDataStringAttr("room", room_);
}

void DataFile::setSampleRate(float sampleRate) 
{
	writeDataAttr("sample-rate", H5::PredType::IEEE_F32LE, &sampleRate);
	sampleRate_ = sampleRate;
}

void DataFile::setGain(float gain) 
{
	writeDataAttr("gain", H5::PredType::IEEE_F32LE, &gain);
	gain_ = gain;
}

void DataFile::setOffset(float offset) 
{
	writeDataAttr("offset", H5::PredType::IEEE_F32LE, &offset);
	offset_ = offset;
}

void DataFile::setDate(std::string date)
{
	writeDataStringAttr("date", date);
	date_ = date;
}

void DataFile::setRoom(std::string room) 
{
	writeDataStringAttr("room", room);
	room_ = room;
}

void DataFile::setArray(std::string array)
{
	writeDataStringAttr("array", array_);
	array_ = array;
}

void DataFile::readFileAttr(std::string name, void *buf) 
{
	try {
		H5::Attribute attr = file.openAttribute(name);
		attr.read(attr.getDataType(), buf);
	} catch (H5::FileIException &e) {
		std::cerr << "File exception accessing attribute: " << name << std::endl;
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	}
}

void DataFile::readDataAttr(std::string name, void *buf) 
{
	try {
		H5::Attribute attr = dataset.openAttribute(name);
		attr.read(attr.getDataType(), buf);
	} catch (H5::DataSetIException &e) {
		std::cerr << "DataSet exception accessing attribute: " << name << std::endl;
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	}
}

void DataFile::readDataStringAttr(std::string name, std::string &loc) 
{
	try {
		H5::Attribute attr = dataset.openAttribute(name);
		hsize_t sz = attr.getStorageSize();
		char *buf = new char[sz + 1]();
		if (buf == NULL)
			throw;
		readDataAttr(name, buf);
		loc.clear();
		loc.resize(sz + 1);
		loc.replace(0, sz, buf);
		delete buf;
	} catch (H5::DataSetIException &e) {
		std::cerr << "DataSet exception accessing attribute: " << name << std::endl;
	} catch (H5::AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	} catch (std::exception &e) {
		std::cerr << "Calloc error" << std::endl;
	}
}

void DataFile::readFileStringAttr(std::string name, std::string &loc) 
{
	try {
		H5::Attribute attr = file.openAttribute(name);
		hsize_t sz = attr.getStorageSize();
		char *buf = new char[sz + 1]();
		if (buf == NULL)
			throw;
		readDataAttr(name, buf);
		loc.clear();
		loc.resize(sz + 1);
		loc.replace(0, sz, buf);
		delete buf;
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
	readDataAttr("sample-rate", &sampleRate_);
}

void DataFile::readGain(void) 
{
	readDataAttr("gain", &gain_);
}

void DataFile::readOffset(void) 
{
	readDataAttr("offset", &offset_);
}

void DataFile::readDate(void) 
{
	readDataStringAttr("date", date_);
}

void DataFile::readRoom(void) 
{
	readDataStringAttr("room", room_);
}

void DataFile::readArray()
{
	readDataStringAttr("array", array_);
}

void DataFile::flush(void) 
{
	file.flush(H5F_SCOPE_GLOBAL);
}

void DataFile::computeCoords(const arma::uvec& channels, 
		int start, int end, arma::Mat<hsize_t>& out, hsize_t* nelem) const
{
	auto nsamp = end - start;
	auto nchan = channels.n_elem;
	*nelem = nsamp * nchan;
	out.set_size(datafile::DATASET_RANK, *nelem);
	for (decltype(nchan) c = 0; c < nchan; c++) {
		for (auto s = 0; s < nsamp; s++) {
			out(0, c * nsamp + s) = channels(c);
			out(1, c * nsamp + s) = s + start;
		}
	}
}

}; // end datafile namespace

