/* hidensfile.cc
 *
 * Implementation of class for reading/writing data in a HiDens recording file.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "hidensfile.h"

#include <sstream>

namespace hidensfile {

HidensFile::HidensFile(std::string filename,
		std::string array, int nchannels)
		: DataFile(filename, array, nchannels)
{
	if (readOnly())
		readConfiguration();
	else {
		setSampleRate(SampleRate);
	}
}

arma::Col<uint32_t> HidensFile::xpos() const { return m_xpos; }
arma::Col<uint32_t> HidensFile::ypos() const { return m_ypos; }
arma::Col<uint16_t> HidensFile::x() const { return m_x; }
arma::Col<uint16_t> HidensFile::y() const { return m_y; }
arma::Col<uint8_t> HidensFile::label() const { return m_label; }
arma::Col<uint32_t> HidensFile::indices() const { return m_indices; }

Configuration HidensFile::configuration() const
{
	return m_configuration;
}

void HidensFile::setConfiguration(const Configuration& config)
{
	m_configuration = config;
	auto sz = m_configuration.size();
	if (m_xpos.size() != sz) {
		m_xpos.resize(sz);
		m_ypos.resize(sz);
		m_x.resize(sz);
		m_y.resize(sz);
		m_label.resize(sz);
		m_indices.resize(sz);
	}
	for (decltype(sz) i = 0; i < sz; i++) {
		auto& val = config[i];
		m_xpos[i] = val.xpos;
		m_ypos[i] = val.ypos;
		m_x[i] = val.x;
		m_y[i] = val.y;
		m_label[i] = val.label;
		m_indices[i] = val.index;
	}
	writeConfiguration();
}

void HidensFile::readConfiguration()
{
	H5::Group grp;
	try {
		grp = m_file.openGroup("configuration");
	} catch (H5::FileIException &e) {
		std::stringstream what;
		what << "The file " << filename() 
				<< " does not have a valid configuration group";
		std::invalid_argument(what.str());
	}
	try {
		auto xposDset = grp.openDataSet("xpos");
		readConfigurationDataset(xposDset, m_xpos);
		auto yposDset = grp.openDataSet("ypos");
		readConfigurationDataset(yposDset, m_ypos);
		auto xDset = grp.openDataSet("x");
		readConfigurationDataset(xDset, m_x);
		auto yDset = grp.openDataSet("y");
		readConfigurationDataset(yDset, m_y);
		auto labelDset = grp.openDataSet("label");
		readConfigurationDataset(labelDset, m_label);
		auto indicesDset = grp.openDataSet("indices");
		readConfigurationDataset(indicesDset, m_indices);

		m_configuration.reserve(m_xpos.n_elem);
		for (arma::uword i = 0; i < m_xpos.n_elem; i++) {
			m_configuration.emplace_back(Electrode{
					m_indices(i),
					m_xpos(i), 
					m_x(i),
					m_ypos(i),
					m_y(i),
					m_label(i)
				});
		}

	} catch (H5::DataSetIException& e) {
		std::stringstream what;
		what << "The file " << filename() <<
			" is missing a configuration dataset";
		std::invalid_argument(what.str());
	}
}

void HidensFile::writeConfiguration()
{
	H5::Group grp;
	try {
		grp = m_file.openGroup("configuration");
	} catch ( ... ) {
		grp = m_file.createGroup("configuration");
		hsize_t rank = 1;
		hsize_t dims[] = { static_cast<hsize_t>(m_xpos.size()) };
		auto space = H5::DataSpace(rank, dims);
		grp.createDataSet("xpos", H5::PredType::STD_U32LE, space);
		grp.createDataSet("ypos", H5::PredType::STD_U32LE, space);
		grp.createDataSet("x", H5::PredType::STD_U16LE, space);
		grp.createDataSet("y", H5::PredType::STD_U16LE, space);
		H5::StrType labelType(0, 1);
		grp.createDataSet("label", labelType, space);
		grp.createDataSet("indices", H5::PredType::STD_U32LE, space);
	}
	try {
		auto xposDset = grp.openDataSet("xpos");
		writeConfigurationDataset(xposDset, m_xpos);
		auto yposDset = grp.openDataSet("ypos");
		writeConfigurationDataset(yposDset, m_ypos);
		auto xDset = grp.openDataSet("x");
		writeConfigurationDataset(xDset, m_x);
		auto yDset = grp.openDataSet("y");
		writeConfigurationDataset(yDset, m_y);
		auto labelDset = grp.openDataSet("label");
		writeConfigurationDataset(labelDset, m_label);
		auto indicesDset = grp.openDataSet("indices");
		writeConfigurationDataset(indicesDset, m_indices);
	} catch (H5::DataSetIException& e) {
		std::stringstream what;
		what << "The file " << filename() <<
			" is missing a configuration dataset";
		std::invalid_argument(what.str());
	}
}

void HidensFile::setAnalogOutputSize(int /* size */)
{
}

} // end hidensfile namespace

