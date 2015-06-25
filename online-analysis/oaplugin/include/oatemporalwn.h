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
const QString OATEMPORALWN_NAME = "Temporal white noise correlation";
const QString OATEMPORALWN_DESCRIPTION = "Computes a reverse-\
correlation between a one-dimensional temporal white noise \
sequence and the selected channel data.";

class OATemporalWN : public QObject, public OAInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID OAINTERFACE_IID)
	Q_INTERFACES(OAInterface)

	public:
		void init(void) Q_DECL_OVERRIDE;
		void run(const uint64_t start, const double rate,
				const arma::vec& data, Stimulus* stim, 
				arma::vec& out) Q_DECL_OVERRIDE;
		void run(const uint64_t start, const double rate,
				const arma::vec& data, Stimulus* stim, 
				arma::mat& out) Q_DECL_OVERRIDE;
		void run(const uint64_t start, const double rate,
				const arma::vec& data, Stimulus* stim, 
				arma::cube& out) Q_DECL_OVERRIDE;

		void get(arma::vec& out) Q_DECL_OVERRIDE;
		void get(arma::mat& out) Q_DECL_OVERRIDE;
		void get(arma::cube& out) Q_DECL_OVERRIDE;

		QString name(void) Q_DECL_OVERRIDE;
		unsigned int ndim(void) Q_DECL_OVERRIDE;
		unsigned int npoints(void) Q_DECL_OVERRIDE;
		QString description(void) Q_DECL_OVERRIDE;

	private:
		arma::vec oa;
};

#endif

