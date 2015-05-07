/* plotworker.h
 * Implementation of class that actually performs plotting of data.
 * This is meant to be run inside a separate thread.
 * 
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _PLOT_WORKER_H_
#define _PLOT_WORKER_H_

#include <QVector>
#include <QSemaphore>

#include "qcustomplot.h"
#include "settings.h"

class PlotWorker : public QObject {
	Q_OBJECT

	public:
		PlotWorker(int id, QObject *parent = 0);
		PlotWorker(const PlotWorker &other);
		~PlotWorker();
	
	public slots:
		void transferPlotData(QSemaphore *sem, int workerId, 
				int channel, QCPGraph *subplot, QVector<double> *data);
		void replot(QSemaphore *sem, const int nthreads, QCustomPlot *p);

	signals:
		void dataTransferDone(int channel);

	private:
		void getSettings(void);
		void constructXData(int npoints);

		int id;
		QPen pen;
		bool transferring;
		bool automean;
		bool autoscale;
		double scale;
		QVector<double> xData;
		Settings settings;
};

#endif

