/* plotworker.cpp
 * Implementation of the worker object that actually plots data
 * in separate threads for efficiency.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "plotworker.h"

PlotWorker::PlotWorker(QSet<int> channels, QObject *parent) : QObject(parent) {
	channelSet = channels;
	getSettings();
	redPen = QPen(Qt::red);
	transferring = false;
}

PlotWorker::PlotWorker(const PlotWorker &other) {
	pen = QPen(other.pen);
	redPen = QPen(other.redPen);
	automean = other.automean;
	autoscale = other.autoscale;
	scale = other.scale;
	xData = QVector<double>(other.xData);
	transferring = false;
}

PlotWorker::~PlotWorker() {
}

void PlotWorker::transferPlotData(QSemaphore *sem, int channel, QString label, 
		QCPGraph *subplot, QVector<double> *data, bool isClicked) {

	if (!channelSet.contains(channel))
		return;

	transferring = true;
	getSettings();
	constructXData(data->size());

	if (automean) {
		double mean = 0.0;
		for (auto i = 0; i < data->size(); i++)
			mean += data->at(i);
		mean /= data->size();
		for (auto i = data->begin(); i != data->end(); i++)
			(*i) -= mean;
	}
	sem->acquire();
	if (isClicked)
		subplot->setPen(redPen);
	else
		subplot->setPen(pen);
	subplot->keyAxis()->setLabel(label);
	subplot->setData(xData, *data);
	if ( (autoscale) || RESCALED_CHANNELS.contains(channel) )
		subplot->valueAxis()->rescale();
	else
		subplot->valueAxis()->setRange(-scale * NEG_DISPLAY_RANGE,
				scale * POS_DISPLAY_RANGE);
	subplot->keyAxis()->rescale();
	sem->release();
	transferring = false;
	delete data;

	emit dataTransferDone(channel);
}

void PlotWorker::getSettings(void) {
	pen = settings.getPlotPen();
	automean = settings.getAutoMean();
	autoscale = settings.getAutoscale();
	scale = settings.getDisplayScale();
}

void PlotWorker::constructXData(int npoints) {
	if (xData.size() == npoints)
		return;
	xData.resize(npoints);
	std::iota(xData.begin(), xData.end(), 0.0);
}

void PlotWorker::replot(QSemaphore *sem, const int nthreads, 
		QCustomPlot *p) {
	sem->acquire(nthreads);
	p->replot();
	sem->release(nthreads);
}

