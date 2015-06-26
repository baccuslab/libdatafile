/* oatemporalwn.cpp
 * ----------------
 *
 * Class implementing the online analysis interface for a temporal 
 * white noise stimulus.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "oatemporalwn.h"

/* This must be the ONLY definition of this global variable */
std::map<std::string, maker_function*> oamap;

void OATemporalWN::init(Stimulus *stim)
{
	oa = arma::vec(OATEMPORALWN_NUM_POINTS, arma::fill::zeros);
}

void OATemporalWN::run(const uint64_t start, const double rate,
		const arma::vec& data, Stimulus* stim, arma::vec& out)
{
	auto fs = 1 / rate;
	auto startTime = start * fs;
	out.set_size(OATEMPORALWN_NUM_POINTS);
	arma::vec tmp(out.n_elem);
	for (auto i = OATEMPORALWN_NUM_POINTS; i < data.n_elem; i++) {
		stim->at(startTime + (i - OATEMPORALWN_NUM_POINTS) * fs,
				rate, OATEMPORALWN_NUM_POINTS, tmp);
		out += tmp * data(i);
	}
	oa += out;
	oa /= oa.max();
	out = oa;
}

void OATemporalWN::run(const uint64_t start, const double rate,
		const arma::vec& data, Stimulus* stim, arma::mat& out)
{
	throw std::invalid_argument(
			"OATemporalWN provides only a 1-dimensional online analysis");
}

void OATemporalWN::run(const uint64_t start, const double rate,
		const arma::vec& data, Stimulus* stim, arma::cube& out)
{
	throw std::invalid_argument(
			"OATemporalWN provides only a 1-dimensional online analysis");
}

std::string OATemporalWN::name(void)
{
	return OATEMPORALWN_NAME;
}

unsigned int OATemporalWN::ndim(void)
{
	return OATEMPORALWN_NUM_DIM;
}

unsigned int OATemporalWN::npoints(void)
{
	return OATEMPORALWN_NUM_POINTS;
}

std::string OATemporalWN::description(void)
{
	return OATEMPORALWN_DESCRIPTION;
}

void OATemporalWN::get(arma::vec& out) 
{
	out = oa;
}

void OATemporalWN::get(arma::mat& out)
{
	throw std::invalid_argument(
			"OATemporalWN provides only a 1-dimensional online analysis");
}

void OATemporalWN::get(arma::cube& out)
{
	throw std::invalid_argument(
			"OATemporalWN provides only a 1-dimensional online analysis");
}

extern "C" 
{
	/* All online analyses must export this function. Change "OATemporalWN"
	 * with the name of the new derived derived class.
	 */
	OAInterface *OATemporalWNMaker() { return new OATemporalWN; }

	/* The Proxy class is a trick to register each class's maker
	 * function with the global oaList, without explicitly notifying
	 * the main application of this class's presence or the name of 
	 * its constructor method.
	 */
	class OATemporalWNProxy 
	{
		public:
			OATemporalWNProxy() { oamap["temporalwn"] = OATemporalWNMaker; }
	};
	OATemporalWNProxy oatemporalwnproxy;
}

