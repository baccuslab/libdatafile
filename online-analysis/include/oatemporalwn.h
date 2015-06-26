/* oatemporalwn.h
 * --------------
 *
 * Definition of plugin class implementing online spike-triggered average
 * computation for a temporal white noise stimulus.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _OATEMPORALWN_H_
#define _OATEMPORALWN_H_

#include "oainterface.h"

const unsigned int OATEMPORALWN_NUM_DIM = 1;
const unsigned int OATEMPORALWN_NUM_POINTS = 30;
const std::string OATEMPORALWN_NAME = "Temporal white noise correlation";
const std::string OATEMPORALWN_DESCRIPTION = "Computes a reverse-\
correlation between a one-dimensional temporal white noise \
sequence and the selected channel data.";

class OATemporalWN : public OAInterface
{
	public:
		void init(Stimulus *stim) override;
		void run(const uint64_t start, const double rate,
				const arma::vec& data, Stimulus* stim, 
				arma::vec& out) override;
		void run(const uint64_t start, const double rate,
				const arma::vec& data, Stimulus* stim, 
				arma::mat& out) override;
		void run(const uint64_t start, const double rate,
				const arma::vec& data, Stimulus* stim, 
				arma::cube& out) override;

		void get(arma::vec& out) override;
		void get(arma::mat& out) override;
		void get(arma::cube& out) override;

		std::string name(void) override;
		unsigned int ndim(void) override;
		unsigned int npoints(void) override;
		std::string description(void) override;

	private:
		arma::vec oa;
};

#endif

