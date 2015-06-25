/* oainterface.h
 * -------------
 *
 * This file describes the interface for any online analysis plugins.
 * Such plugins should subclass both QObject and this interface, and 
 * implement its functionality.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _OAINTERFACE_H_
#define _OAINTERFACE_H_

#define OAINTERFACE_IID "org.baccuslab.mearec.oainterface"

#include <QtPlugin>
#include <QString>
#include <armadillo>

#include "stimulus.h"

class OAInterface
{
	public:
		virtual ~OAInterface() { };
		virtual void init(void) = 0;
		virtual void run(const uint64_t start, const double rate, 
				const arma::vec& data, Stimulus* stim, arma::vec& out) = 0;
		virtual void run(const uint64_t start, const double rate, 
				const arma::vec& data, Stimulus* stim, arma::mat& out) = 0;
		virtual void run(const uint64_t start, const double rate, 
				const arma::vec& data, Stimulus* stim, arma::cube& out) = 0;
		virtual void get(arma::vec& out) = 0;
		virtual void get(arma::mat& out) = 0;
		virtual void get(arma::cube& out) = 0;

		virtual QString name(void) = 0;
		virtual unsigned int npoints(void) = 0;
		virtual unsigned int ndim(void) = 0;
		virtual QString description(void) = 0;

};

Q_DECLARE_INTERFACE( OAInterface, OAINTERFACE_IID )

#endif

