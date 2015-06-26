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

#include <map>
#include <string>
#include <armadillo>
#include "stimulus.h"

/* The OAInterface class is the virutal base class for any online
 * analysis plugins. These derived classes must implement all methods,
 * and provide a maker function that wraps the class's constructor.
 */
class OAInterface
{
	public:
		virtual ~OAInterface() { };
		virtual void init(Stimulus *stim) = 0;
		virtual void run(const uint64_t start, const double rate, 
				const arma::vec& data, Stimulus* stim, arma::vec& out) = 0;
		virtual void run(const uint64_t start, const double rate, 
				const arma::vec& data, Stimulus* stim, arma::mat& out) = 0;
		virtual void run(const uint64_t start, const double rate, 
				const arma::vec& data, Stimulus* stim, arma::cube& out) = 0;
		virtual void get(arma::vec& out) = 0;
		virtual void get(arma::mat& out) = 0;
		virtual void get(arma::cube& out) = 0;

		virtual std::string name(void) = 0;
		virtual unsigned int npoints(void) = 0;
		virtual unsigned int ndim(void) = 0;
		virtual std::string description(void) = 0;

};

/* `oamap` is a global variable containing the list of all
 * known online analyses. Users must register their custom derived classes
 * by adding them to the list. See ../src/oatemporalwn.cpp for an example.
 */
typedef OAInterface *maker_function();
extern std::map<std::string, maker_function*> oamap;

#endif

