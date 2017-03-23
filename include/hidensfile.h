/* hidensfile.h
 *
 * Implementation of simple subclass of the DataFile class, which represents
 * a file for data recorded from the HiDens array.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef MEAREC_HIDENSFILE_H_
#define MEAREC_HIDENSFILE_H_

#include "datafile.h"
#include "configuration.h"

namespace hidensfile {

/*! The default number of channels in a HiDens recording */
const int NumChannels = 126;

/*! The HiDens data sample rate */
const float SampleRate = 20000;

/*! The default array type */
const std::string DefaultArray = "hidens";

/*! The HidensFile class is a Datafile subclass that provides extra functionality
 * specific to HiDens array recordings.
 */
class HidensFile : public datafile::DataFile {
	public:

		/*! Construct a HiDens recording file. */
		HidensFile(std::string filename, 
				std::string array = DefaultArray,
				int nchannels = NumChannels);

		/*! Return the configuration saved in this file */
		Configuration configuration() const;

		/*! Return the list of x-positions for all connected electrodes */
		arma::Col<uint32_t> xpos() const;

		/*! Return the list of y-positions for all connected electrodes */
		arma::Col<uint32_t> ypos() const;

		/*! Return the list of x-indices for all connected electrodes */
		arma::Col<uint16_t> x() const;

		/*! Return the list of y-indices for all connected electrodes */
		arma::Col<uint16_t> y() const;

		/*! Return the list of labels for all connected electrodes */
		arma::Col<uint8_t> label() const;

		/*! Return the indices of all connected electrodes */
		arma::Col<uint32_t> indices() const;

		/*! Write the given configuration into the file */
		void setConfiguration(const Configuration&);

		/*! Override for the Hidens file class which enforces 
		 * that setting analog output is not supported for this
		 * class.
		 */
		virtual void setAnalogOutputSize(int sz) override;

	protected:
		void readConfiguration();
		void writeConfiguration();

		/* Configuration itself. */
		Configuration m_configuration;
		arma::Col<uint32_t> m_xpos, m_ypos;
		arma::Col<uint16_t> m_x, m_y;
		arma::Col<uint8_t> m_label;
		arma::Col<uint32_t> m_indices;

		/* Read components of each Electrode struct.  */
		template<class T>
		void readConfigurationDataset(const H5::DataSet& dset, arma::Col<T>& out) const
		{
			/* Define file (source) space. */
			auto space = dset.getSpace();
			hsize_t dims[1] = { 0 };
			space.getSimpleExtentDims(dims);
			auto sz = dims[0];
			hsize_t offset[1] = { 0 };
			hsize_t count[1] = { sz };
			space.selectHyperslab(H5S_SELECT_SET, count, offset);

			/* Create memory (destination) dataspace */
			auto memspace = H5::DataSpace(1, dims);
			memspace.selectHyperslab(H5S_SELECT_SET, count, offset);

			/* Read into output array */
			out.set_size(sz);
			dset.read(out.memptr(), datafile::dtypeForMat(out), memspace, space);
		}

		/* Specialization for labels. 
		 *
		 * The datatype in memory is uint8_t, which should just be directly 
		 * convertible from a single-byte char. But HDF's string datatypes are
		 * more complicated, and must be specified as full StrType's to be
		 * read correctly.
		 */
		void readConfigurationDataset(const H5::DataSet& dset, 
				arma::Col<uint8_t>& out) const
		{
			/* Define file (source) space. */
			auto space = dset.getSpace();
			hsize_t dims[1] = { 0 };
			space.getSimpleExtentDims(dims);
			auto sz = dims[0];
			hsize_t offset[1] = { 0 };
			hsize_t count[1] = { sz };
			space.selectHyperslab(H5S_SELECT_SET, count, offset);

			/* Create memory (destination) dataspace */
			auto memspace = H5::DataSpace(1, dims);
			memspace.selectHyperslab(H5S_SELECT_SET, count, offset);

			/* Read into output array */
			out.set_size(sz);
			H5::StrType type{ H5::PredType::C_S1, 1 };
			dset.read(out.memptr(), type, memspace, space);
		}

		/* Write components of each Electrode struct.  */
		template<class T>
		void writeConfigurationDataset(H5::DataSet& dset, const arma::Col<T>& in)
		{
			/* Define file (destination) dataspace. */
			hsize_t dims[1] = { in.n_elem };
			H5::DataSpace space(1, dims);
			hsize_t offset[1] = { 0 };
			space.selectHyperslab(H5S_SELECT_SET, dims, offset);

			/* Create memory (source) dataspace. */
			auto memspace = H5::DataSpace(1, dims);
			memspace.selectHyperslab(H5S_SELECT_SET, dims, offset);

			/* Write from input array */ 
			dset.write(in.memptr(), datafile::dtypeForMat(in), memspace, space);
		}

}; // end HidensFile class
}; // end hidensfile namespace

#endif

