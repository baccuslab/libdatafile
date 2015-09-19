/* hidensfile.tc
 *
 * Implementation of templated functions to read and write
 * elements of the configuration in a HiDens data file.
 * 
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <typeinfo>

template<class T>
void hidensfile::HidensFile::readConfigurationDataset(const H5::DataSet& dset, T& out)
{
	H5::DataSpace space = dset.getSpace();
	hsize_t dims[1] = {0};
	space.getSimpleExtentDims(dims);
	auto sz = dims[0];
	hsize_t offset[1] = {0};
	hsize_t count[1] = {sz};
	space.selectHyperslab(H5S_SELECT_SET, count, offset);

	auto memspace = H5::DataSpace(1, dims);
	memspace.selectHyperslab(H5S_SELECT_SET, count, offset);
	out.set_size(sz);
	H5::DataType dtype;
	auto hash = typeid(T).hash_code();
	if (hash == typeid(arma::Col<uint32_t>).hash_code())
		dtype = H5::PredType::STD_U32LE;
	else if (hash == typeid(arma::Col<uint16_t>).hash_code())
		dtype = H5::PredType::STD_U16LE;
	else if (hash == typeid(arma::Col<uint8_t>).hash_code()) {
		auto strtype = H5::StrType(H5::PredType::C_S1, 1);
		strtype.setStrpad(dset.getStrType().getStrpad());
		dtype = strtype;
	} else if (hash == typeid(arma::Col<int32_t>).hash_code())
		dtype = H5::PredType::STD_I32LE;
	dset.read(out.memptr(), dtype, memspace, space);
}

template<class T>
void hidensfile::HidensFile::writeConfigurationDataset(H5::DataSet& dset, T& in)
{
	hsize_t dims[1] = {in.n_elem};
	H5::DataSpace space(1, dims);
	hsize_t offset[1] = {0};
	space.selectHyperslab(H5S_SELECT_SET, dims, offset);

	auto memspace = H5::DataSpace(1, dims);
	memspace.selectHyperslab(H5S_SELECT_SET, dims, offset);
	H5::DataType dtype;
	auto hash = typeid(T).hash_code();
	if (hash == typeid(arma::Col<uint32_t>).hash_code())
		dtype = H5::PredType::STD_U32LE;
	else if (hash == typeid(arma::Col<uint16_t>).hash_code())
		dtype = H5::PredType::STD_U16LE;
	else if (hash == typeid(arma::Col<uint8_t>).hash_code()) {
		auto strtype = H5::StrType(0, 1);
		strtype.setStrpad(H5T_STR_NULLTERM);
		dtype = strtype;
	} else if (hash == typeid(arma::Col<int32_t>).hash_code())
		dtype = H5::PredType::STD_I32LE;
	dset.write(in.memptr(), dtype, memspace, space);
}
