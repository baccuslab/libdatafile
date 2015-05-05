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
#include <QtConcurrent>

/* Other third-party includes */
#include "qcustomplot.h"

#include "h5recording/include/h5recording.h"

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
		//void plotData(QVector<QVector<int16_t> > data);
		void plotData(QVector<QVector<double> > &data);
		void clear();

		QVector<double> xData;
		void distributedPlot(H5Rec::Samples &s);
		void plotSubBlock(H5Rec::Samples &s, int block);
		void distributedPlotData(QVector<QVector<double> > &data);
		void plotDataSubBlock(QVector<QVector<double> > &data, int i);

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

