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

void PlotWindow::plotData(samples &s) {
	this->channelPlot->distributedPlot(s);
}

void PlotWindow::plotNextDataBlock() {

	//QVector<QVector<int16_t> > data = this->playback->getNextDataBlock();
	//qDebug() << "Data read start: " << QTime::currentTime();
	

	//if (this->playback != nullptr) 
	//QVector<QVector<double> > data = this->playback->getNextDataBlock();
	//else
		//QVector<QVector<double> > data = this->recording->getNextDataBlock();


	//qDebug() << "Data read end: " << QTime::currentTime();
	//QtConcurrent::run(this->channelPlot, &ChannelPlot::plotData, data);
	//this->channelPlot->distributedPlotData(data);
	

}

void PlotWindow::toggleVisible() {
	this->setVisible(!this->isVisible());
}

ChannelPlot *PlotWindow::getChannelPlot() {
	return this->channelPlot;
}
