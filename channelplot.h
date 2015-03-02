/* channelplot.h
 * Header file for a single channel's data plot in the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _CHANNEL_PLOT_H_
#define _CHANNEL_PLOT_H_

/* C++ includes */
#include <memory>			// Smart pointers

/* Qt includes */
#include <QString>

/* Other third-party includes */
#include "qcustomplot.h"

/* meaview includes */
#include "config.h"

using namespace std;

class ChannelPlot : public QCustomPlot {
	Q_OBJECT

	public:
		ChannelPlot(int, pos_t, QWidget *parent = 0);
		int getChannelIndex();
		pos_t getPosition();

	private:
		/* Methods */
		void setTitle();

		/* Attributes */
		int channel;
		pos_t position;
		QString titleString;
		unique_ptr<QCPPlotTitle> title;
};

#endif

