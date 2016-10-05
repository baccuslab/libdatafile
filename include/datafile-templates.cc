/* datafile.tc
 *
 * Implementation of templated functions for reading and writing data
 * to HDF5 recording files.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

 #include <typeinfo>
 #include <sstream>

template<class T>
void datafile::DataFile::data(int channel, int startSample, 
		int endSample, T& data) const
{
	datafile::DataFile::data(channel, channel + 1, startSample, 
			endSample, data);
}

template<class T>
void datafile::DataFile::data(int startChannel, int endChannel, int startSample, 
		int endSample, arma::Mat<T>& data) const
{
	/* Check input */
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
	if (requestedSamples < 0) {
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
	if (requestedChannels < 0) {
		throw std::logic_error("Requested sample range invalid: (" + 
				std::to_string(startSample) + "-" + 
				std::to_string(endSample) + ")");
	}

	/* Select hyperslab from data set itself */
	hsize_t spaceOffset[datafile::DATASET_RANK] = {
			static_cast<hsize_t>(startChannel),
			static_cast<hsize_t>(startSample)
		};
	hsize_t spaceCount[datafile::DATASET_RANK] = {
			static_cast<hsize_t>(requestedChannels),
			static_cast<hsize_t>(requestedSamples)
		};
	dataspace.selectHyperslab(H5S_SELECT_SET, spaceCount, spaceOffset);
	if (!dataspace.selectValid()) {
		std::stringstream what;
		what << "Dataset selection invalid:" << std::endl
				<< "Offset: (" << startSample << ", 0)" << std::endl
				<< "Count: (" << requestedSamples << ", "
				<< nchannels() << ")" << std::endl;
		throw std::logic_error(what.str());
	}

	/* Define the destination data space in memory */
	hsize_t dims[datafile::DATASET_RANK] = {
			static_cast<hsize_t>(requestedChannels),
			static_cast<hsize_t>(requestedSamples)
		};
	H5::DataSpace memspace(datafile::DATASET_RANK, dims);
	hsize_t offset[datafile::DATASET_RANK] = {0, 0};
	hsize_t count[datafile::DATASET_RANK] = {
			static_cast<hsize_t>(requestedChannels),
			static_cast<hsize_t>(requestedSamples)
		};
	memspace.selectHyperslab(H5S_SELECT_SET, count, offset);
	if (!memspace.selectValid()) {
		std::stringstream what;
		what << "Memory dataspace selection invalid:" << std::endl
				<< "Count: (" << requestedSamples << ", "
				<< nchannels() << ")" << std::endl;
		throw std::logic_error(what.str());
	}

	/* Get the datatype of the array into which memory is read */
	H5::DataType datatype;
	auto hash = typeid(T).hash_code();
	if (hash == typeid(double).hash_code()) {
		datatype = H5::PredType::IEEE_F64LE;
		data.set_size(requestedSamples, requestedChannels);
	} else if (hash == typeid(short).hash_code()) {
		datatype = H5::PredType::STD_I16LE;
		data.set_size(requestedSamples, requestedChannels);
	} else if (hash == typeid(uint8_t).hash_code()) {
		datatype = H5::PredType::STD_U8LE;
		data.set_size(requestedSamples, requestedChannels);
	} 
	dataset.read(data.memptr(), datatype, memspace, dataspace);
}

template<class T> 
void datafile::DataFile::data(const arma::uvec& channels,
		const int start, const int end, T& data) const
{
	/* Verify input and resize return array */
	if (end <= start) {
		std::cerr << "Requested sample range is invalid: Samples" 
			<< start << " - " << end << std::endl;
		throw std::logic_error("Requested sample range invalid");
	}
	size_t nreqSamples = end - start;
	size_t nreqChannels = channels.n_elem;
	arma::Mat<hsize_t> coords;
	hsize_t nelem;
	computeCoords(channels, start, end, coords, &nelem);
	data.set_size(nreqSamples, nreqChannels);

	/* Select hyperslab from the file */
	dataspace.selectElements(H5S_SELECT_SET, nelem, coords.memptr());
	if (!dataspace.selectValid()) {
		std::cerr << "Dataset selection invalid" << std::endl;
		std::cerr << "Offset: (0, " << start << ")" << std::endl;
		std::cerr << "Count: (" << nchannels() << ", " << nreqSamples << ")" << std::endl;
		throw std::logic_error("Dataset selection invalid");
	}

	/* Define memory dataspace */
	hsize_t mdims[datafile::DATASET_RANK] = {nreqChannels, nreqSamples};
	H5::DataSpace mspace(datafile::DATASET_RANK, mdims);
	hsize_t moffset[datafile::DATASET_RANK] = {0, 0};
	hsize_t mcount[datafile::DATASET_RANK] = {nreqChannels, nreqSamples};
	mspace.selectHyperslab(H5S_SELECT_SET, mcount, moffset);
	if (!mspace.selectValid()) {
		std::cerr << "Memory dataspace selection invalid" << std::endl;
		std::cerr << "Count: (" << nreqSamples << ", " << nreqChannels << ")" << std::endl;
		throw std::logic_error("Memory dataspace selection invalid");
	}

	/* Get datatype of memory data space and read */
	H5::DataType datatype;
	auto hash = typeid(T).hash_code();
	if (hash == typeid(double).hash_code())
		datatype = H5::PredType::IEEE_F64LE;
	else if (hash == typeid(short).hash_code())
		datatype = H5::PredType::STD_I16LE;
	else if (hash == typeid(uint8_t).hash_code())
		datatype = H5::PredType::STD_U8LE;
	dataset.read(data.memptr(), datatype, mspace, dataspace);
}

