/* oalineswn.cpp
 * -------------
 *
 * Class implementing online analysis interface for 1D spatiotemporal
 * white noise lines.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "oalineswn.h"

void OALinesWN::init(Stimulus *stim)
{
	oa = arma::mat(stim->nx(), OALINESWN_NUM_POINTS);
}

void OALinesWN::run(const uint64_t start, const double rate,
		const arma::vec& data, Stimulus *stim, arma::vec& out)
{
	throw std::invalid_argument(
			"OALinesWN provides only a 2-dimensional online analysis");
}

void OALinesWN::run(const uint64_t start, const double rate,
		const arma::vec& data, Stimulus *stim, arma::mat& out)
{
	auto fs = 1 / rate;
	auto startTime = start * fs;
	out.set_size(stim->nx(), OALINESWN_NUM_POINTS);
	arma::mat tmp(out.n_rows, out.n_cols);
	for (auto i = OALINESWN_NUM_POINTS; i < data.n_elem; i++) {
		stim->at(startTime + (i - OALINESWN_NUM_POINTS) * fs,
				rate, OALINESWN_NUM_POINTS, tmp);
		out += tmp * data(i);
	}
	oa += out;
	oa /= oa.max();
	out = oa;
}

void OALinesWN::run(const uint64_t start, const double rate,
		const arma::vec& data, Stimulus *stim, arma::cube& out)
{
	throw std::invalid_argument(
			"OALinesWN provides only a 2-dimensional online analysis");
}

std::string OALinesWN::name(void)
{
	return OALINESWN_NAME;
}

std::string OALinesWN::description(void)
{
	return OALINESWN_DESCRIPTION;
}

unsigned int OALinesWN::ndim(void)
{
	return OALINESWN_NUM_NIM;
}

unsigned int OALinesWN::npoints(void)
{
	return OALINESWN_NUM_POINTS;
}

void OALinesWN::get(arma::vec& out)
{
	throw std::invalid_argument(
			"OALinesWN provides only a 2-dimensional online analysis");
}

void OALinesWN::get(arma::mat& out)
{
	out = oa;
}

void OALinesWN::get(arma::cube& out)
{
	throw std::invalid_argument(
			"OALinesWN provides only a 2-dimensional online analysis");
}

extern "C"
{
	OAInterface *OALinesWNMaker() { return new OALinesWN; }
	
	class OALinesWNProxy
	{
		public:
			OALinesWNProxy() { oamap["lineswn"] = OALinesWNMaker; }
	};
	OALinesWNProxy oalineswnproxy;
}
