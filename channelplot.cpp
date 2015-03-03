/* channelplot.cpp
 * Implementation of a single channel's data plot for the meaview appliccation.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

/* C++ includes */

#if QT_VERSION < 0x050000
/* Qt includes */
#include <QFont>
#endif

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
	this->xAxis->grid()->setVisible(false);
	this->yAxis->setTicks(false);
	this->yAxis->setTickLabels(false);
	this->yAxis->grid()->setVisible(false);
	this->setTitle();
	this->replot();
}

void ChannelPlot::setTitle() {
	this->titleString = QString("%1%2").arg(this->position.row).arg(this->position.col);
	this->plotLayout()->insertRow(0);
	this->title = new QCPPlotTitle(this, this->titleString);
	this->title->setFont(QFont("Helvetica", -1, QFont::Light));
	this->plotLayout()->addElement(0, 0, this->title);
}

int ChannelPlot::getChannelIndex() {
	return this->channel;
}

pos_t ChannelPlot::getPosition() {
	return this->position;
}

