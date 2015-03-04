/* channelplot.h
 * Header file for a single channel's data plot in the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _CHANNEL_PLOT_H_
#define _CHANNEL_PLOT_H_

/* C++ includes */
//#include <memory>			// Smart pointers

/* Qt includes */
#include <QString>
#include <QFont>

/* Other third-party includes */
#include "qcustomplot.h"

/* meaview includes */
#include "settings.h"

using namespace std;

class ChannelPlot : public QCustomPlot {
	Q_OBJECT

	public:
		ChannelPlot(int, QPair<int, int>, QWidget *parent = 0);
		int getChannelIndex();
		QPair<int, int> &getPosition();

	private:
		/* Methods */
		void setTitle();

		/* Attributes */
		Settings settings;
		int channel;
		QPair<int, int> position;
		QString titleString;
		QCPPlotTitle *title;
};

#endif

