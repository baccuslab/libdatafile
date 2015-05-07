/* channelinspector.cpp
 * Implementation of ChannelInspector class, allowing a zoomed-in
 * view of data from a single channel
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "channelinspector.h"
#include "h5recording/include/h5recording.h"

ChannelInspector::ChannelInspector(QCustomPlot *parentPlot,
		QCPGraph *sourceGraph, int chan, QWidget *parent) : 
		QWidget(parent, Qt::Window) {

	channel = chan;
	this->sourceGraph = sourceGraph;

	/* Create plot axis, graph, etc. */
	plot = new QCustomPlot(this);
	graph = plot->addGraph();
	graph->keyAxis()->setTicks(false);
	graph->keyAxis()->setTickLabels(false);
	graph->keyAxis()->grid()->setVisible(false);
	graph->keyAxis()->setRange(0, H5Rec::SAMPLE_RATE *
			settings.getRefreshInterval() / 1000);
	graph->valueAxis()->setTicks(false);
	graph->valueAxis()->setTickLabels(false);
	graph->valueAxis()->grid()->setVisible(false);
	graph->setPen(settings.getPlotPen());

	/* Copy data */
	QCPDataMap *data = sourceGraph->data();
	graph->setData(data, true);
	graph->rescaleValueAxis();
	plot->replot();

	/* Add to window */
	layout = new QGridLayout(this);
	layout->addWidget(plot);
	setLayout(layout);
	setWindowTitle(QString("Mealog: Channel %1").arg(channel));
	setAttribute(Qt::WA_DeleteOnClose);
	resize(INSPECTOR_WINDOW_WIDTH, INSPECTOR_WINDOW_HEIGHT);
	connect(parentPlot, &QCustomPlot::afterReplot, 
			this, &ChannelInspector::replot);
}

ChannelInspector::~ChannelInspector() {
}

void ChannelInspector::replot() {
	graph->setData(sourceGraph->data(), true);
	graph->rescaleValueAxis();
	plot->replot();
}

