/* h5recording.cpp
 * Implementation of class representing a single MEA recording and
 * the HDF5 file to which it is saved
 */

#include <sys/stat.h>
#include <iostream>
#include <stdlib.h>
#include <ctime>

#include "h5recording.h"

using namespace H5;
using namespace H5Rec;

H5Recording::H5Recording(std::string filename) {
	filename_ = filename;

	/* If file exists, verify it is valid HDF5 and load data from it.
	 * Else, construct a new file.
	 */
	struct stat buffer;
	if (stat(filename_.c_str(), &buffer) == 0) {
		if (!H5File::isHdf5(filename_)) {
			std::cerr << "Invalid H5 file" << std::endl;
			throw std::invalid_argument("Invalid HDF5 file");
		}
		try {
			readOnly = true;
			file = H5File(filename_, H5F_ACC_RDONLY);
		} catch (FileIException &e) {
			std::cerr << "Could not open H5 file" << std::endl;
			throw std::runtime_error("Could not open HDF5 file");
		}
		/* Get the data itself */
		try {
			dataset = file.openDataSet("data");
		} catch (FileIException &e) {
			std::cerr << "File must contain a dataset labeled 'data'";
			throw std::invalid_argument("File must contain a 'data' dataset");
		}
		dataspace = dataset.getSpace();

		/* Read attributes into data members */
		try {
			readIsLive();
			readLastValidSample();
			readFileType();
			readSampleRate();
			readBlockSize();
			readNumSamples();
			readNumChannels();
			readGain();
			readOffset();
			readDate();
			readRoom();
		} catch ( ... ) {
			std::cerr << "File does not contain appropriate attributes";
			throw std::invalid_argument(
					"H5 file does not contain appropriate attributes");
		}

		/* Compute length of experiment */
		setLength(nsamples() / sampleRate());

	} else {
		/* New file. This needs to be only called by mealog... */
		readOnly = false;

		/* Construct the file. Define to have a chunk cache large enough to hold
		 * a few chunks at a time.
		 */
		FileAccPropList fileProps = FileAccPropList(FileAccPropList::DEFAULT);
		int mdc_nelmts = 0;
		size_t rdcc_nelmts, rdcc_nbytes = 0;
		size_t chunkCacheSizeElems = (
				H5Rec::CHUNK_CACHE_SIZE * H5Rec::BLOCK_SIZE * H5Rec::NUM_CHANNELS
			);
		double rdcc_w0 = 0.0;
		fileProps.getCache(mdc_nelmts, rdcc_nelmts, rdcc_nbytes, rdcc_w0);
		fileProps.setCache(mdc_nelmts, chunkCacheSizeElems, 
				chunkCacheSizeElems * sizeof(int16_t), rdcc_w0);
		file = H5File(filename_, H5F_ACC_TRUNC, 
				FileCreatPropList::DEFAULT, fileProps);

		/* Create the dataset */
		dataspace = DataSpace(H5Rec::DATASET_RANK, H5Rec::DATASET_DEFAULT_DIMS, 
				H5Rec::DATASET_MAX_DIMS);
		props = DSetCreatPropList();
		props.setChunk(H5Rec::DATASET_RANK, H5Rec::DATASET_CHUNK_DIMS);
		datatype = DataType(PredType::STD_I16LE);
		dataset = file.createDataSet("data", datatype, dataspace, props);

		/* Set default parameters */
		setFileType(H5Rec::BIN_FILE_TYPE);
		setFileVersion(H5Rec::BIN_FILE_VERSION);
		setNumChannels(H5Rec::NUM_CHANNELS);
		setBlockSize(H5Rec::BLOCK_SIZE);
		setSampleRate(H5Rec::SAMPLE_RATE);
		setRoom(H5Rec::DEFAULT_ROOM_STRING);
		setDate();
	}
}

H5Recording::~H5Recording() {
	try {
		if (!readOnly) 
			writeAllAttributes();
		file.close();
	} catch (FileIException &e) {
		std::cerr << "Error closing HDF5 file: " << filename_ << std::endl;
	}
}

std::string H5Recording::filename(void) {
	return filename_;
}

double H5Recording::length(void) {
	return length_;
}

void H5Recording::setLength(double length) {
	length_ = length;
	setNumSamples(length_ * H5Rec::SAMPLE_RATE);

	/* If this is a new recording, it will not be read-only. Requests
	 * to re-set the length of the file will be done by a class that
	 * is recording the data to disk.
	 */
	if (readOnly)
		return;
	hsize_t newSize[H5Rec::DATASET_RANK] = {H5Rec::NUM_CHANNELS, nsamples_};
	dataset.extend(newSize);
	dataspace = dataset.getSpace();
}

int16_t H5Recording::type(void) {
	return type_;
}

int16_t H5Recording::version(void) {
	return version_;
}

uint32_t H5Recording::nsamples(void) {
	return nsamples_;
}

uint32_t H5Recording::nchannels(void) {
	return nchannels_;
}

bool H5Recording::live(void) {
	return live_;
}

uint32_t H5Recording::lastValidSample(void) {
	return lastValidSample_;
}

uint32_t H5Recording::blockSize(void) {
	return blockSize_;
}

float H5Recording::sampleRate(void) {
	return sampleRate_;
}

float H5Recording::gain(void) {
	return gain_;
}

float H5Recording::offset(void) {
	return offset_;
}

std::string H5Recording::date(void) {
	return date_;
}

std::string H5Recording::room(void) {
	return room_;
}

H5Rec::Samples H5Recording::data(int startSample, int endSample) {
	/* Allocate return array */
	int req_nsamples = endSample - startSample;
	if (req_nsamples < 0) {
		std::cerr << "Requested sample range is invalid: (" << 
				startSample << ", " << endSample << ")" << std::endl;
		throw std::logic_error("Requested sample range invalid");
	}
	H5Rec::Samples s(req_nsamples, nchannels_);
	data(startSample, endSample, s);
	return s;
}

void H5Recording::data(int startSample, int endSample, H5Rec::Samples &s) {
	int req_nsamples = endSample - startSample;
	if (req_nsamples < 0) {
		std::cerr << "Requested sample range is invalid: (" << 
				startSample << ", " << endSample << ")" << std::endl;
		throw std::logic_error("Requested sample range invalid");
	}

	/* Select hyperslab from data set itself */
	hsize_t space_offset[H5Rec::DATASET_RANK] = {0, 
			static_cast<hsize_t>(startSample)};
	hsize_t space_count[H5Rec::DATASET_RANK] = {nchannels_, 
			static_cast<hsize_t>(req_nsamples)};
	dataspace.selectHyperslab(H5S_SELECT_SET, space_count, space_offset);
	if (!dataspace.selectValid()) {
		std::cerr << "Dataset selection invalid: " << std::endl;
		std::cerr << "Offset: (" << startSample << ", 0)" << std::endl;
		std::cerr << "Count: (" << req_nsamples << ", " << nchannels_ << ")" << std::endl;
		throw std::logic_error("Dataset selection invalid");
	}

	/* Define dataspace of memory region, which is contiguous
	 * data chunk of Armadillo matrix and its hyperslab.
	 */
	hsize_t dims[H5Rec::DATASET_RANK] = {nchannels_,
			static_cast<hsize_t>(req_nsamples)};
	DataSpace memspace(H5Rec::DATASET_RANK, dims);
	hsize_t mem_offset[H5Rec::DATASET_RANK] = {0, 0};
	hsize_t mem_count[H5Rec::DATASET_RANK] = {nchannels_,
			static_cast<hsize_t>(req_nsamples)};
	memspace.selectHyperslab(H5S_SELECT_SET, mem_count, mem_offset);
	if (!memspace.selectValid()) {
		std::cerr << "Memory dataspace selection invalid: " << std::endl;
		std::cerr << "Count: (" << req_nsamples << ", " << nchannels_ << ")" << std::endl;
		throw std::logic_error("Memory dataspace selection invalid");
	}

	/* Read data */
	dataset.read(s.memptr(), PredType::STD_I16LE, memspace, dataspace);
}

void H5Recording::data(int startSample, int endSample, H5Rec::SamplesD &s) {
	int req_nsamples = endSample - startSample;
	if (req_nsamples < 0) {
		std::cerr << "Requested sample range is invalid: (" << 
				startSample << ", " << endSample << ")" << std::endl;
		throw std::logic_error("Requested sample range invalid");
	}

	/* Select hyperslab from data set itself */
	hsize_t space_offset[H5Rec::DATASET_RANK] = {0, 
			static_cast<hsize_t>(startSample)};
	hsize_t space_count[H5Rec::DATASET_RANK] = {nchannels_, 
			static_cast<hsize_t>(req_nsamples)};
	dataspace.selectHyperslab(H5S_SELECT_SET, space_count, space_offset);
	if (!dataspace.selectValid()) {
		std::cerr << "Dataset selection invalid: " << std::endl;
		std::cerr << "Offset: (" << startSample << ", 0)" << std::endl;
		std::cerr << "Count: (" << req_nsamples << ", " << nchannels_ << ")" << std::endl;
		throw std::logic_error("Dataset selction invalid");
	}

	/* Define dataspace of memory region, which is contiguous
	 * data chunk of Armadillo matrix and its hyperslab.
	 */
	hsize_t dims[H5Rec::DATASET_RANK] = {nchannels_,
			static_cast<hsize_t>(req_nsamples)};
	DataSpace memspace(H5Rec::DATASET_RANK, dims);
	hsize_t mem_offset[H5Rec::DATASET_RANK] = {0, 0};
	hsize_t mem_count[H5Rec::DATASET_RANK] = {nchannels_,
			static_cast<hsize_t>(req_nsamples)};
	memspace.selectHyperslab(H5S_SELECT_SET, mem_count, mem_offset);
	if (!memspace.selectValid()) {
		std::cerr << "Memory dataspace selection invalid: " << std::endl;
		std::cerr << "Count: (" << req_nsamples << ", " << nchannels_ << ")" << std::endl;
		throw std::logic_error("Memory dataspace selection invalid");
	}

	/* Read data */
	dataset.read(s.memptr(), PredType::IEEE_F64LE, memspace, dataspace);

	/* Scale and offset by ADC properties */
	s = s * gain_ + offset_;
}

arma::vec H5Recording::data(int startSample, int endSample, int channel) {
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
	data(startSample, endSample, channel, s);
	return s;
}

void H5Recording::data(int startSample, int endSample, int channel, 
		arma::vec& data) 
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

	/* Select hyperslab from data set itself */
	hsize_t space_offset[H5Rec::DATASET_RANK] = {static_cast<hsize_t>(channel), 
			static_cast<hsize_t>(startSample)};
	hsize_t space_count[H5Rec::DATASET_RANK] = {1, 
			static_cast<hsize_t>(req_nsamples)};
	dataspace.selectHyperslab(H5S_SELECT_SET, space_count, space_offset);
	if (!dataspace.selectValid()) {
		std::cerr << "Dataset selection invalid: " << std::endl;
		std::cerr << "Offset: (" << startSample << ", 0)" << std::endl;
		std::cerr << "Count: (" << req_nsamples << ", " << nchannels_ << ")" << std::endl;
		throw std::logic_error("Dataset selection invalid");
	}

	/* Define dataspace of memory region, which is contiguous
	 * data chunk of Armadillo matrix and its hyperslab.
	 */
	hsize_t dims[H5Rec::DATASET_RANK] = {1,
			static_cast<hsize_t>(req_nsamples)};
	DataSpace memspace(H5Rec::DATASET_RANK, dims);
	hsize_t mem_offset[H5Rec::DATASET_RANK] = {0, 0};
	hsize_t mem_count[H5Rec::DATASET_RANK] = {1,
			static_cast<hsize_t>(req_nsamples)};
	memspace.selectHyperslab(H5S_SELECT_SET, mem_count, mem_offset);
	if (!memspace.selectValid()) {
		std::cerr << "Memory dataspace selection invalid: " << std::endl;
		std::cerr << "Count: (" << req_nsamples << ", " << nchannels_ << ")" << std::endl;
		throw std::logic_error("Memory dataspace selection invalid");
	}

	/* Read data */
	dataset.read(data.memptr(), PredType::IEEE_F64LE, memspace, dataspace);
	data = data * gain() + offset();
}

void H5Recording::data(int startSample, int endSample, int channel, 
		arma::Col<short>& data)
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

	/* Select hyperslab from data set itself */
	hsize_t space_offset[H5Rec::DATASET_RANK] = {static_cast<hsize_t>(channel), 
			static_cast<hsize_t>(startSample)};
	hsize_t space_count[H5Rec::DATASET_RANK] = {1, 
			static_cast<hsize_t>(req_nsamples)};
	dataspace.selectHyperslab(H5S_SELECT_SET, space_count, space_offset);
	if (!dataspace.selectValid()) {
		std::cerr << "Dataset selection invalid: " << std::endl;
		std::cerr << "Offset: (" << startSample << ", 0)" << std::endl;
		std::cerr << "Count: (" << req_nsamples << ", " << nchannels_ << ")" << std::endl;
		throw std::logic_error("Dataset selection invalid");
	}

	/* Define dataspace of memory region, which is contiguous
	 * data chunk of Armadillo matrix and its hyperslab.
	 */
	hsize_t dims[H5Rec::DATASET_RANK] = {1,
			static_cast<hsize_t>(req_nsamples)};
	DataSpace memspace(H5Rec::DATASET_RANK, dims);
	hsize_t mem_offset[H5Rec::DATASET_RANK] = {0, 0};
	hsize_t mem_count[H5Rec::DATASET_RANK] = {1,
			static_cast<hsize_t>(req_nsamples)};
	memspace.selectHyperslab(H5S_SELECT_SET, mem_count, mem_offset);
	if (!memspace.selectValid()) {
		std::cerr << "Memory dataspace selection invalid: " << std::endl;
		std::cerr << "Count: (" << req_nsamples << ", " << nchannels_ << ")" << std::endl;
		throw std::logic_error("Memory dataspace selection invalid");
	}

	/* Read data */
	dataset.read(data.memptr(), PredType::STD_I16LE, memspace, dataspace);
}

void H5Recording::setFilename(std::string filename) {
	filename_ = filename;
}

void H5Recording::setData(int startSample, int endSample, H5Rec::Samples &data) {
	int req_nsamples = endSample - startSample;
	if (req_nsamples <= 0) {
		std::cerr << "Requested sample range is invalid: (" <<
				startSample << ", " << endSample << ")" << std::endl;
		throw std::logic_error("Requested sample range invalid");
	}

	/* Select hyperslab of dataspace where data will be written */
	hsize_t space_offset[H5Rec::DATASET_RANK] = {0, 
			static_cast<hsize_t>(startSample)};
	hsize_t space_count[H5Rec::DATASET_RANK] = {nchannels_,
			static_cast<hsize_t>(req_nsamples)};
	dataspace.selectHyperslab(H5S_SELECT_SET, space_count, space_offset);
	if (!dataspace.selectValid()) {
		std::cerr << "Dataset selection invalid: " << std::endl;
		std::cerr << "Offset: (" << startSample << ", 0)" << std::endl;
		std::cerr << "Count: (" << req_nsamples << ", " << nchannels_ << ")" << std::endl;
		throw std::logic_error("Dataset selection invalid");
	}

	/* Define dataspace of memory region, from which data is read */
	hsize_t dims[H5Rec::DATASET_RANK] = {nchannels_, 
			static_cast<hsize_t>(req_nsamples)};
	DataSpace memspace(H5Rec::DATASET_RANK, dims);
	hsize_t mem_offset[H5Rec::DATASET_RANK] = {0, 0};
	hsize_t mem_count[H5Rec::DATASET_RANK] = {nchannels_, 
			static_cast<hsize_t>(req_nsamples)};
	memspace.selectHyperslab(H5S_SELECT_SET, mem_count, mem_offset);
	if (!memspace.selectValid()) {
		std::cerr << "Memory dataspace selection invalid: " << std::endl;
		std::cerr << "Count: (" << req_nsamples << ", " << nchannels_ << ")" << std::endl;
		throw std::logic_error("Memory dataspace selection invalid");
	}

	/* Write data */
	dataset.write(data.memptr(), PredType::STD_I16LE, memspace, dataspace);
	flush();
}

void H5Recording::writeFileAttr(std::string name, const DataType &type, void *buf) {
	if (readOnly)
		return;
	try {
		DataType writeType(type);
		if (!(file.attrExists(name))) {
			DataSpace space(H5S_SCALAR);
			file.createAttribute(name, type, space);
		}
		Attribute attr = file.openAttribute(name);
		attr.write(writeType, buf);
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	} catch (FileIException &e) {
		std::cerr << "File exception accessing: " << name << std::endl;
	}
}

void H5Recording::writeDataAttr(std::string name, const DataType &type, void *buf) {
	if (readOnly)
		return;
	try {
		DataType writeType(type);
		if (!(dataset.attrExists(name))) {
			DataSpace space(H5S_SCALAR);
			dataset.createAttribute(name, writeType, space);
		}
		Attribute attr = dataset.openAttribute(name);
		attr.write(writeType, buf);
	} catch (DataSetIException &e) {
		std::cerr << "DataSet exception accessing: " << name << std::endl;
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	}
}

void H5Recording::writeDataStringAttr(std::string name, std::string value) {
	if (readOnly)
		return;
	try {
		StrType stringType(0, value.length());
		if (!(dataset.attrExists(name))) {
			DataSpace space(H5S_SCALAR);
			dataset.createAttribute(name, stringType, space);
		}
		Attribute attr = dataset.openAttribute(name);
		attr.write(stringType, value.c_str());
	} catch (DataSetIException &e) {
		std::cerr << "DataSet exception accessing: " << name << std::endl;
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing: " << name << std::endl;
	}
}

void H5Recording::writeAllAttributes(void) {
	writeFileAttr("is-live", PredType::STD_U8LE, &live_);
	writeFileAttr("last-valid-sample", 
			PredType::STD_U32LE, &lastValidSample_);
	writeDataAttr("bin-file-type", PredType::STD_I16LE, &type_);
	writeDataAttr("bin-file-version", PredType::STD_I16LE, &version_);
	writeDataAttr("sample-rate", PredType::IEEE_F32LE, &sampleRate_);
	writeDataAttr("block-size", PredType::STD_U32LE, &blockSize_);
	writeDataAttr("gain", PredType::IEEE_F32LE, &gain_);
	writeDataAttr("offset", PredType::IEEE_F32LE, &offset_);
	writeDataStringAttr("date", date_);
	writeDataStringAttr("room", room_);
}

void H5Recording::setLive(bool live) {
	writeFileAttr("is-live", PredType::STD_U8LE, &live);
	live_ = live;
}

void H5Recording::setLastValidSample(uint32_t sample) {
	writeFileAttr("last-valid-sample", PredType::STD_U32LE, &sample);
	lastValidSample_ = sample;
}

void H5Recording::setFileType(int16_t type) {
	writeDataAttr("bin-file-type", PredType::STD_I16LE, &type);
	type_ = type;
}

void H5Recording::setFileVersion(int16_t version) {
	writeDataAttr("bin-file-version", PredType::STD_I16LE, &version);
	version_ = version;
}

void H5Recording::setSampleRate(float sampleRate) {
	writeDataAttr("sample-rate", PredType::IEEE_F32LE, &sampleRate);
	sampleRate_ = sampleRate;
}

void H5Recording::setNumChannels(uint32_t nchannels) {
	nchannels_ = nchannels;
}

void H5Recording::setNumSamples(uint32_t nsamples) {
	nsamples_ = nsamples;
}

void H5Recording::setGain(float gain) {
	writeDataAttr("gain", PredType::IEEE_F32LE, &gain);
	gain_ = gain;
}

void H5Recording::setOffset(float offset) {
	writeDataAttr("offset", PredType::IEEE_F32LE, &offset);
	offset_ = offset;
}

void H5Recording::setBlockSize(size_t blockSize) {
	writeDataAttr("block-size", PredType::STD_U32LE, &blockSize);
	blockSize_ = blockSize;
}

void H5Recording::setDate(void) {
	size_t isoLength = 19;
	const char fmt[] = "%Y-%m-%dT%H:%M:%S";
	std::string date(isoLength + 1, '\0');
	std::time_t t = std::time(nullptr);
	std::strftime(&date[0], isoLength, fmt, std::localtime(&t));
	writeDataStringAttr("date", date);
	date_ = date;
}

void H5Recording::setRoom(std::string room) {
	writeDataStringAttr("room", room);
	room_ = room;
}

void H5Recording::readFileAttr(std::string name, void *buf) {
	try {
		Attribute attr = file.openAttribute(name);
		attr.read(attr.getDataType(), buf);
	} catch (FileIException &e) {
		std::cerr << "File exception accessing attribute: " << name << std::endl;
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	}
}

void H5Recording::readDataAttr(std::string name, void *buf) {
	try {
		Attribute attr = dataset.openAttribute(name);
		attr.read(attr.getDataType(), buf);
	} catch (DataSetIException &e) {
		std::cerr << "DataSet exception accessing attribute: " << name << std::endl;
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	}
}

void H5Recording::readDataStringAttr(std::string name, std::string &loc) {
	try {
		Attribute attr = dataset.openAttribute(name);
		hsize_t sz = attr.getStorageSize();
		char *buf = (char *) calloc(sz + 1, 1);
		if (buf == NULL)
			throw;
		readDataAttr(name, buf);
		loc.replace(0, sz, buf);
		free(buf);
	} catch (DataSetIException &e) {
		std::cerr << "DataSet exception accessing attribute: " << name << std::endl;
	} catch (AttributeIException &e) {
		std::cerr << "Attribute exception accessing attribute: " << name << std::endl;
	} catch (std::exception &e) {
		std::cerr << "Calloc error" << std::endl;
	}
}

void H5Recording::readIsLive(void) {
	readFileAttr("is-live", &live_);
}

void H5Recording::readLastValidSample(void) {
	readFileAttr("last-valid-sample", &lastValidSample_);
}

void H5Recording::readFileType(void) {
	readDataAttr("bin-file-type", &type_);
}

void H5Recording::readFileVersion(void) {
	readDataAttr("bin-file-version", &version_);
}

void H5Recording::readSampleRate(void) {
	readDataAttr("sample-rate", &sampleRate_);
}

void H5Recording::readBlockSize(void) {
	readDataAttr("block-size", &blockSize_);
}

void H5Recording::readNumSamples(void) {
	hsize_t dims[H5Rec::DATASET_RANK] = {0, 0};
	dataset.getSpace().getSimpleExtentDims(dims, NULL);
	nsamples_ = dims[1];
}

void H5Recording::readNumChannels(void) {
	hsize_t dims[H5Rec::DATASET_RANK] = {0, 0};
	dataset.getSpace().getSimpleExtentDims(dims, NULL);
	nchannels_ = dims[0];
}

void H5Recording::readGain(void) {
	readDataAttr("gain", &gain_);
}

void H5Recording::readOffset(void) {
	readDataAttr("offset", &offset_);
}

void H5Recording::readDate(void) {
	readDataStringAttr("date", date_);
}

void H5Recording::readRoom(void) {
	readDataStringAttr("room", room_);
}

void H5Recording::flush(void) {
	file.flush(H5F_SCOPE_GLOBAL);
}

