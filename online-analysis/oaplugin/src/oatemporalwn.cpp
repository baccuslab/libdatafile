/* oatemporalwn.cpp
 * ----------------
 *
 * Class implementing the online analysis interface for a temporal 
 * white noise stimulus.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <iostream>
#include <algorithm>
#include "oatemporalwn.h"

void OATemporalWN::init(void)
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
		//std::cout << "Looking at: " << i << " ( " << 
			//startTime + (i - OATEMPORALWN_NUM_POINTS) * fs << "), data = " <<
			//data(i) << std::endl;
		stim->at(startTime + (i - OATEMPORALWN_NUM_POINTS) * fs,
				rate, OATEMPORALWN_NUM_POINTS, tmp);
		out += tmp * data(i);
	}
	oa += out;
	oa /= oa.max();
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

QString OATemporalWN::name(void)
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

QString OATemporalWN::description(void)
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

