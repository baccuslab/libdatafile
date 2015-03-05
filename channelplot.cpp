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
	for (int chan = 0; chan < NUM_CHANNELS; chan++) {

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
		graph->keyAxis()->setRange(0, 20000);
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
	xData.resize(SAMPLE_RATE *
			this->settings.getRefreshInterval() / 1000);
	constructXData();
}

void ChannelPlot::constructXData() {
	int newSize = SAMPLE_RATE * this->settings.getRefreshInterval() / 1000;
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

void ChannelPlot::plotData(QVector<QVector<int16_t> > data) {
	QPen pen = this->settings.getPlotPen();
	double scale = this->settings.getDisplayScale();
	QVector<double> plotData(data.at(0).size());

	for (auto i = 0; i < data.size(); i++) {
		for(auto j = 0; j < data.at(0).size(); j++)
			plotData[j] = data.at(i).at(j);

		QCPGraph *graph = getSubplot(i);
		graph->setData(xData, plotData);

		if (this->settings.getAutoscale())
			graph->valueAxis()->rescale();
		else
			graph->valueAxis()->setRange(\
					-scale * NEG_DISPLAY_RANGE, scale * POS_DISPLAY_RANGE);
		graph->setPen(pen);
	}
	this->replot();
}

inline int ChannelPlot::posToIndex(int row, int col) {
	QPair<int, int> tmp(row, col);
	return this->settings.getChannelView().indexOf(tmp);
}

//void ChannelPlot::setTitle() {
	//this->titleString = QString("%1%2").arg(
			//this->position.first).arg(this->position.second);
	//this->plotLayout()->insertRow(0);
	//this->title = new QCPPlotTitle(this, this->titleString);
	//this->title->setFont(QFont("Helvetica", -1, QFont::Light));
	//this->plotLayout()->addElement(0, 0, this->title);
//}

//int ChannelPlot::getChannelIndex() {
	//return this->channel;
//}

//QPair<int, int> &ChannelPlot::getPosition() {
	//return this->position;
//}

