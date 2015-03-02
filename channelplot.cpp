/* channelplot.cpp
 * Implementation of a single channel's data plot for the meaview appliccation.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

/* C++ includes */

/* Qt includes */
#include <QFont>

/* meaview includes */
#include "channelplot.h"

using namespace std;

ChannelPlot::ChannelPlot(
		int channel,
		pos_t position,
		QWidget *parent) : QCustomPlot(parent) {

	/* Save channel index and position */
	this->channel = channel;
	this->position = position;

	/* Customize the plot itself */
	this->addGraph();
	this->xAxis->setTicks(false);
	this->xAxis->setTickLabels(false);
	this->yAxis->setTicks(false);
	this->yAxis->setTickLabels(false);
	this->setTitle();
}

void ChannelPlot::setTitle() {
	this->titleString = QString("%1%2").arg(this->position.row).arg(this->position.col);
	this->plotLayout()->insertRow(0);
	this->title = unique_ptr<QCPPlotTitle>(new QCPPlotTitle(this, this->titleString));
	this->title->setFont(QFont("Helvetica", -1, QFont::Light));
}

int ChannelPlot::getChannelIndex() {
	return this->channel;
}

pos_t ChannelPlot::getPosition() {
	return this->position;
}

