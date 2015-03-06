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
		ChannelPlot(int numRows, int numCols, QWidget *parent = 0);
		QCPGraph *getSubplot(int index);
		QCPGraph *getSubplot(int row, int col);
		QCPAxisRect *getSubplotAxis(int index);
		QCPAxisRect *getSubplotAxis(int row, int col);
		void plotData(QVector<QVector<int16_t> > data);

		QVector<double> xData;

	signals:
		void subplotDoubleClicked(int);

	public slots:
		void constructXData();

	private:
		/* Methods */
		int posToIndex(int row, int col);
		void mouseDoubleClickEvent(QMouseEvent *);
		int findRectIndex(QPoint);

		/* Attributes */
		Settings settings;

};

#endif

