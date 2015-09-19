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

namespace hidensfile {

const int NUM_CHANNELS = 126;
const float SAMPLE_RATE = 20000;
const std::string DEFAULT_ARRAY("hidens-v2");

typedef struct {
	int32_t channel;
	uint32_t xpos;
	uint32_t ypos;
	uint16_t x;
	uint16_t y;
	uint8_t label;
} Electrode;

class HidensFile : public datafile::DataFile {
	public:
		HidensFile(std::string filename, 
				std::string array = DEFAULT_ARRAY,
				int nchannels = NUM_CHANNELS);

		std::vector<Electrode> configuration() const;
		arma::Col<uint32_t> xpos() const;
		arma::Col<uint32_t> ypos() const;
		arma::Col<uint16_t> x() const;
		arma::Col<uint16_t> y() const;
		arma::Col<uint8_t> label() const;
		arma::Col<int32_t> channels() const;

		void setConfiguration(const std::vector<Electrode>&);

	protected:
		void readConfiguration();
		void writeConfiguration();

		/* Read/write components of each Electrode struct.
		 * See hidensfile.tc for implemenation
		 */
		template<class T>
		void readConfigurationDataset(const H5::DataSet& dset, T& out);
		template<class T>
		void writeConfigurationDataset(H5::DataSet& dset, T& out);

		std::vector<Electrode> configuration_;
		arma::Col<uint32_t> xpos_, ypos_;
		arma::Col<uint16_t> x_, y_;
		arma::Col<uint8_t> label_;
		arma::Col<int32_t> channels_;

}; // end HidensFile class
}; // end hidensfile namespace

#include "hidensfile-templates.cc"

#endif

