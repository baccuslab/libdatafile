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
		: datafile::DataFile(filename, array)
{
	if (readOnly) {
		readConfiguration();
	} else {
		setNumChannels(nchannels);
	}
}

arma::Col<uint32_t> HidensFile::xpos() const { return xpos_; }
arma::Col<uint32_t> HidensFile::ypos() const { return ypos_; }
arma::Col<uint16_t> HidensFile::x() const { return x_; }
arma::Col<uint16_t> HidensFile::y() const { return y_; }
arma::Col<uint8_t> HidensFile::label() const { return label_; }
arma::Col<int32_t> HidensFile::channels() const { return channels_; }

std::vector<Electrode> HidensFile::configuration() const
{
	return configuration_;
}

void HidensFile::setConfiguration(const std::vector<Electrode>& config)
{
	configuration_ = config;
	auto sz = configuration_.size();
	if (xpos_.size() != sz) {
		xpos_.resize(sz);
		ypos_.resize(sz);
		x_.resize(sz);
		y_.resize(sz);
		label_.resize(sz);
		channels_.resize(sz);
	}
	for (auto i = 0; i < sz; i++) {
		auto& val = config[i];
		xpos_[i] = val.xpos;
		ypos_[i] = val.ypos;
		x_[i] = val.x;
		y_[i] = val.y;
		label_[i] = val.label;
		channels_[i] = val.channel;
	}
	writeConfiguration();
}

void HidensFile::readConfiguration()
{
	H5::Group grp;
	try {
		grp = file.openGroup("configuration");
	} catch (H5::FileIException &e) {
		std::stringstream what;
		what << "The file " << filename() 
				<< " does not have a valid configuration group";
		std::invalid_argument(what.str());
	}
	try {
		auto xposDset = grp.openDataSet("xpos");
		readConfigurationDataset(xposDset, xpos_);
		auto yposDset = grp.openDataSet("ypos");
		readConfigurationDataset(yposDset, ypos_);
		auto xDset = grp.openDataSet("x");
		readConfigurationDataset(xDset, x_);
		auto yDset = grp.openDataSet("y");
		readConfigurationDataset(yDset, y_);
		auto labelDset = grp.openDataSet("label");
		readConfigurationDataset(labelDset, label_);
		auto channelDset = grp.openDataSet("channels");
		readConfigurationDataset(channelDset, channels_);
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
		grp = file.openGroup("configuration");
	} catch (H5::FileIException &e) {
		grp = file.createGroup("configuration");
	}
	try {
		auto xposDset = grp.openDataSet("xpos");
		readConfigurationDataset(xposDset, xpos_);
		auto yposDset = grp.openDataSet("ypos");
		readConfigurationDataset(yposDset, ypos_);
		auto xDset = grp.openDataSet("x");
		readConfigurationDataset(xDset, x_);
		auto yDset = grp.openDataSet("y");
		readConfigurationDataset(yDset, y_);
		auto labelDset = grp.openDataSet("label");
		readConfigurationDataset(labelDset, label_);
		auto channelDset = grp.openDataSet("channels");
		readConfigurationDataset(channelDset, channels_);
	} catch (H5::DataSetIException& e) {
		std::stringstream what;
		what << "The file " << filename() <<
			" is missing a configuration dataset";
		std::invalid_argument(what.str());
	}
}

}; // end hidensfile namespace

