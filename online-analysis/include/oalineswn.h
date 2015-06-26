/* oalineswn.h
 * -----------
 *
 * Definition of plugin class implementing online reverse-correlation
 * for a one-dimensional spatiotemporal white noise stimulus, i.e.,
 * white noise lines.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _OALINESWN_H_
#define _OALINESWN_H_

#include "oainterface.h"

const unsigned int OALINESWN_NUM_NIM = 2;
const unsigned int OALINESWN_NUM_POINTS = 30;
const std::string OALINESWN_NAME = "1D spatiotemporal noise reverse-correlation";
const std::string OALINESWN_DESCRIPTION = "Computes a reverse-"\
		"correlation between a one-dimensional spatiotemporal white noise"\
		" stimulus (white noise lines) and the selected channel data.";

class OALinesWN : public OAInterface
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
		arma::mat oa;
};

#endif

