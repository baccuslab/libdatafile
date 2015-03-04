/* channelplot.cpp
 * Implementation of a single channel's data plot for the meaview appliccation.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

/* meaview includes */
#include "channelplot.h"

using namespace std;

ChannelPlot::ChannelPlot(
		int channel,
		QPair<int, int> position,
		QWidget *parent) : QCustomPlot(parent) {

	/* Save channel index and position */
	this->channel = channel;
	this->position = position;

	/* Customize the plot itself */
	this->addGraph();
	this->graph(0)->setPen(settings.getPlotPen());
	this->yAxis->setRange(-NEG_DISPLAY_RANGE, POS_DISPLAY_RANGE);
	this->xAxis->setRange(0, 20000);
	this->xAxis->setTicks(false);
	this->xAxis->setTickLabels(false);
	this->xAxis->grid()->setVisible(false);
	this->yAxis->setTicks(false);
	this->yAxis->setTickLabels(false);
	this->yAxis->grid()->setVisible(false);
	this->setTitle();
	this->replot();
}

void ChannelPlot::setTitle() {
	this->titleString = QString("%1%2").arg(
			this->position.first).arg(this->position.second);
	this->plotLayout()->insertRow(0);
	this->title = new QCPPlotTitle(this, this->titleString);
	this->title->setFont(QFont("Helvetica", -1, QFont::Light));
	this->plotLayout()->addElement(0, 0, this->title);
}

int ChannelPlot::getChannelIndex() {
	return this->channel;
}

QPair<int, int> &ChannelPlot::getPosition() {
	return this->position;
}

