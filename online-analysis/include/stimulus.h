/* stimulus.h
 * ----------
 *
 * Class representing a stimulus used in online analyses.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _STIMULUS_H_
#define _STIMULUS_H_

#include <string>
#include <armadillo>
#include "H5Cpp.h"

class Stimulus {
	public:
		Stimulus(std::string filename, std::string stimname = "stimulus");
		Stimulus(const Stimulus& other) = delete;
		~Stimulus();

		/* Various overloaded frames for returning stimulus frames */
		arma::mat frame(unsigned int frame);
		arma::cube frames(unsigned int start, unsigned int end);
		void frames(unsigned int start, unsigned int end, arma::vec& out);
		void frames(unsigned int start, unsigned int end, arma::mat& out);
		void frames(unsigned int start, unsigned int end, arma::cube& out);

		/* The various "at" methods return frames as a function of time,
		 * rather than index. Given a start time, a desired sample rate, and
		 * a number of points, they return the stimulus frames at those time
		 * points.
		 */
		void at(const arma::vec& points, arma::vec& out);
		void at(const arma::vec& points, arma::mat& out);
		void at(const arma::vec& points, arma::cube& out);
		void at(double startTime, double rate, unsigned int N, arma::vec& out);
		void at(double startTime, double rate, unsigned int N, arma::mat& out);
		void at(double startTime, double rate, unsigned int N, arma::cube& out);

		hsize_t ndim(void) { return ndim_; };
		hsize_t nx(void);		// Return number of pixels in x
		hsize_t ny(void);		// Return number of pixels in y
		hsize_t nframes(void);	// Return number of frames in stimulus

		hsize_t frameBefore(double t);	// Frame immediately preceding given time
		double frameRate(void) { return trueRate; };
		unsigned int nominalRate(void) { return frate; };

	private:
		double trueRate;			// "True" frame rate
		unsigned int frate;			// Frame rate used in computations (rounded)
		double ifi;					// Inter-frame interval, from nominal rate
		hsize_t ndim_;				// Number of dimensions > 1
		hsize_t *dims;				// Array of dimensions
		std::string filename;		// Name of stimulus file
		H5::H5File file;			// H5File object
		H5::DataSet stimulus;		// Stimulus dataset
		H5::DataSpace stimSpace;	// Data space for stimulus

		void frames_(unsigned int, unsigned int, double *);
};

#endif

