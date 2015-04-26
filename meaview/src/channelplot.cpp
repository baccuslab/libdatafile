/* channelplot.cpp
 * Implementation of a single channel's data plot for the meaview appliccation.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

/* meaview includes */
#include "channelplot.h"

using namespace std;

ChannelPlot::ChannelPlot(int numRows, int numCols, QWidget *parent) 
		: QCustomPlot(parent) {
	/* Set up the subplots */
	this->plotLayout()->removeAt(0);
	this->plotLayout()->expandTo(numRows, numCols);
	this->plotLayout()->setRowSpacing(-10);
	this->plotLayout()->setColumnSpacing(-10);
	for (int chan = 0; chan < H5Rec::NUM_CHANNELS; chan++) {

		/* Create axes and graph for the subplot */
		QPair<int, int> pos = this->settings.getChannelView().at(chan);
		QCPAxisRect *rect = new QCPAxisRect(this);
		this->plotLayout()->addElement(pos.first, pos.second, rect);
		QCPGraph *graph = new QCPGraph(
				rect->axis(QCPAxis::atBottom), rect->axis(QCPAxis::atLeft));
		this->addPlottable(graph);

		/* Style the plot */
		graph->keyAxis()->setTicks(false);
		graph->keyAxis()->setTicks(false);
		graph->keyAxis()->setTickLabels(false);
		graph->keyAxis()->grid()->setVisible(false);
		graph->keyAxis()->setRange(0, H5Rec::SAMPLE_RATE * 
				this->settings.getRefreshInterval() / 1000);
		graph->valueAxis()->setTicks(false);
		graph->valueAxis()->setTickLabels(false);
		graph->valueAxis()->grid()->setVisible(false);
		graph->valueAxis()->setRange(
				-(this->settings.getDisplayScale() * NEG_DISPLAY_RANGE), 
				(this->settings.getDisplayScale() * POS_DISPLAY_RANGE));
	}
	
	/* Make an x-axis for all subplots. This is not const, really, since
	 * we expect that the user may change the refresh rate of the plots
	 */
	xData.resize(H5Rec::SAMPLE_RATE *
			this->settings.getRefreshInterval() / 1000);
	constructXData();
}

void ChannelPlot::constructXData() {
	int newSize = H5Rec::SAMPLE_RATE * this->settings.getRefreshInterval() / 1000;
	if (xData.size() != newSize)
		xData.resize(newSize);
	double start = 0.0;
	iota(xData.begin(), xData.end(), start);
}

QCPGraph *ChannelPlot::getSubplot(int index) {
	return this->graph(index);
}

QCPGraph *ChannelPlot::getSubplot(int row, int col) {
	return this->graph(posToIndex(row, col));
}

QCPAxisRect *ChannelPlot::getSubplotAxis(int index) {
	return this->axisRect(index);
}

QCPAxisRect *ChannelPlot::getSubplotAxis(int row, int col) {
	return this->axisRect(posToIndex(row, col));
}

void ChannelPlot::distributedPlot(H5Rec::samples &s) {
	this->constructXData();
	qDebug() << "X data size: " << xData.size();
	for (auto i = 0; i < 8; i++)
		QtConcurrent::run(this, &ChannelPlot::plotSubBlock, s, i);
}

void ChannelPlot::plotSubBlock(H5Rec::samples &s, int block) {
	QPen pen = this->settings.getPlotPen();
	bool automean = this->settings.getAutoMean();
	bool autoscale = this->settings.getAutoscale();
	double scale = this->settings.getDisplayScale();
	QVector<double> plotData(s.shape()[1]);
	for (auto j = 8 * block ; j < 8 * (block + 1); j++) {
		QCPGraph *graph = getSubplot(j);
		if (automean) {
			double mean = 0.0;
			for (auto k = 0; k < plotData.size(); k++)
				mean += s[j][k];
			mean /= s.shape()[1];
			for (auto k = 0; k < plotData.size(); k++) 
				plotData[k] = s[j][k] - mean;
			graph->setData(xData, plotData);
		} else {
			for (auto k = 0; k < plotData.size(); k++)
				plotData[k] = s[j][k];
			graph->setData(xData, plotData);
		}
		graph->setPen(pen);

		if ((autoscale) || RESCALED_CHANNELS.contains(j))
			graph->valueAxis()->rescale();
		else
			graph->valueAxis()->setRange(-scale * NEG_DISPLAY_RANGE,
					scale * POS_DISPLAY_RANGE);
		graph->keyAxis()->rescale();
	}
	this->replot();
}

void ChannelPlot::distributedPlotData(QVector<QVector<double> > &data) {
	for (auto i = 0; i < 8; i++)
		QtConcurrent::run(this, &ChannelPlot::plotDataSubBlock, data, i);
}

void ChannelPlot::plotDataSubBlock(QVector<QVector<double> > &data, int block) {
	qDebug() << "Plotting sub-block " << block << "in thread" << QThread::currentThread();
	QPen pen = this->settings.getPlotPen();
	bool automean = this->settings.getAutoMean();
	bool autoscale = this->settings.getAutoscale();
	double scale = this->settings.getDisplayScale();
	QVector<double> plotData(data.at(0).size());
	for (auto j = 8 * block ; j < 8 * (block + 1); j++) {
		QCPGraph *graph = getSubplot(j);
		if (automean) {
			double mean = 0.0;
			for (auto k = 0; k < plotData.size(); k++)
				mean += data.at(j).at(k);
			mean /= data.at(0).size();
			for (auto k = 0; k < plotData.size(); k++) 
				plotData[k] = data.at(j).at(k) - mean;
			graph->setData(xData, plotData);
		} else {
			graph->setData(xData, data.at(j));
		}
		graph->setPen(pen);

		if ((autoscale) || RESCALED_CHANNELS.contains(j))
			graph->valueAxis()->rescale();
		else
			graph->valueAxis()->setRange(-scale * NEG_DISPLAY_RANGE,
					scale * POS_DISPLAY_RANGE);
	}
	this->replot();
}

//void ChannelPlot::plotData(QVector<QVector<int16_t> > data) {
void ChannelPlot::plotData(QVector<QVector<double> > &data) {
	QPen pen = this->settings.getPlotPen();
	double scale = this->settings.getDisplayScale();

	//qDebug() << "Starting data transfer at:" << QTime::currentTime();
	QVector<double> plotData(data.at(0).size());
	for (auto i = 0; i < data.size(); i++) {
		QCPGraph *graph = getSubplot(i);
		qDebug() << "Channel" << i << "transfer start: " << QTime::currentTime();
		if (settings.getAutoMean()) {
			double mean = 0.0;
			for (auto j = 0; j < data.at(0).size(); j++)
				mean += data.at(i).at(j);
			mean /= data.at(0).size();
			for (auto j = 0; j < data.at(0).size(); j++)
				plotData[j] = data.at(i).at(j) - mean;




		} else {
			//for (auto j = 0; j < data.at(0).size(); j++)
				//plotData[j] = data.at(i).at(j);
			
			/* XXX: If not auto-meaning, no need to copy data to new
			 * array plotData, just use array already in `data`. 
			 */
			graph->setData(xData, data.at(i));
		}


		// Implementation 1
		//graph->setData(xData, plotData);
		//

		// Implementation 2
		//QCPDataMap *newMap = new QCPDataMap();
		//for (auto j = 0; j < data.at(i).size(); j++)
			//newMap->insert(j, QCPData(j, plotData.at(j)));
		//graph->setData(newMap);
		//
		
		qDebug() << "Channel" << i << "transfer end: " << QTime::currentTime();

		if (this->settings.getAutoscale() || RESCALED_CHANNELS.contains(i))
			graph->valueAxis()->rescale();
		else
			graph->valueAxis()->setRange(\
					-scale * NEG_DISPLAY_RANGE, scale * POS_DISPLAY_RANGE);
		graph->setPen(pen);
	}
	//qDebug() << "Switch transfer to render:" << QTime::currentTime();
	this->replot();
	//qDebug() << "Done rendering: " << QTime::currentTime();
}

inline int ChannelPlot::posToIndex(int row, int col) {
	QPair<int, int> tmp(row, col);
	return this->settings.getChannelView().indexOf(tmp);
}

int ChannelPlot::findRectIndex(QPoint p) {
	QList<QCPAxisRect *> rects = this->axisRects();
	for (auto i = 0; i < rects.size(); i++) {
		if (rects.at(i)->rect().contains(p))
			return i;
	}
	return -1;
}

void ChannelPlot::mouseDoubleClickEvent(QMouseEvent *event) {
	qDebug() << "Position: " << event->globalPos();
	int index = findRectIndex(event->globalPos());
	if (index == -1)
		return;
	qDebug() << "Index: " << index;
	//qDebug() << "Inner rect: " << this->axisRects().at(index)->rect();
	//qDebug() << "Outer rect: " << this->axisRects().at(index)->outerRect();
	qDebug() << "(x,y) = " << event->x() << event->y() << endl;
	emit subplotDoubleClicked(index);
}

