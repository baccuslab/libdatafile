/* plotwindow.cpp
 * --------------
 * Implementation of the data plotting window for the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "plotwindow.h"

PlotWindow::PlotWindow(QWidget *parent) : QWidget(parent, Qt::Window) {
	setGeometry(0, 0, PLOT_WINDOW_WIDTH, PLOT_WINDOW_HEIGHT);
	setWindowTitle("Meaview: Channel view");
	initPlotGroup();
}

PlotWindow::~PlotWindow() {
}

void PlotWindow::initPlotGroup() {
	channelPlot = new ChannelPlot(settings.getNumRows(),
			settings.getNumCols(), this);
	layout = new QHBoxLayout();
	layout->addWidget(channelPlot);
	this->setLayout(layout);
}

void PlotWindow::plotNextDataBlock() {
	QVector<QVector<int16_t> > data = this->recording->getNextDataBlock();
	QtConcurrent::run(this->channelPlot, &ChannelPlot::plotData, data);
}

void PlotWindow::toggleVisible() {
	this->setVisible(!this->isVisible());
}
