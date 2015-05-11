/* plotworker.h
 * Implementation of class that actually performs plotting of data.
 * This is meant to be run inside a separate thread.
 * 
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _PLOT_WORKER_H_
#define _PLOT_WORKER_H_

#include <QVector>
#include <QString>
#include <QSet>
#include <QSemaphore>

#include "qcustomplot.h"
#include "settings.h"

class PlotWorker : public QObject {
	Q_OBJECT

	public:
		PlotWorker(QSet<int> channelSet, QObject *parent = 0);
		PlotWorker(const PlotWorker &other);
		~PlotWorker();
	
	public slots:
		void transferPlotData(QSemaphore *sem, int channel, QString label, 
				QCPGraph *subplot, QVector<double> *data, bool isClicked);
		void replot(QSemaphore *sem, const int nthreads, QCustomPlot *p);

	signals:
		void dataTransferDone(int channel);

	private:
		void getSettings(void);
		void constructXData(int npoints);

		QSet<int> channelSet;
		QPen pen;
		QPen redPen;
		bool transferring;
		bool automean;
		bool autoscale;
		double scale;
		QVector<double> xData;
		Settings settings;
};

#endif

