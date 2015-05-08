/* plotwindow.cpp
 * --------------
 * Implementation of the data plotting window for the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <QPair>
#include <QFont>
#include <QStringList>
#include "plotwindow.h"
#include "channelinspector.h"

PlotWindow::PlotWindow(int numRows, int numCols, QWidget *parent) : 
		QWidget(parent, Qt::Window) {
	nrows = numRows;
	ncols = numCols;
	sem = new QSemaphore(numThreads);
	setGeometry(0, 0, PLOT_WINDOW_WIDTH, PLOT_WINDOW_HEIGHT);
	setWindowTitle("Mealog: Channel view");
	initThreadPool();
	initPlotGroup();
	qRegisterMetaType<QSemaphore *>("QSemaphore *");
}

PlotWindow::~PlotWindow() {
	for (auto &worker : workerList)
		delete worker;
	for (auto &thread : threadList) {
		thread->quit();
		thread->wait();
		delete thread;
	}
}

void PlotWindow::initThreadPool() {
	numWorkersPerThread = H5Rec::NUM_CHANNELS / numThreads;
	for (auto i = 0; i < numThreads; i++) {
		threadList.append(new QThread());
		for (auto j = 0; j < numWorkersPerThread; j++) {
			int workerNum = i * numWorkersPerThread + j;
			workerList.append(new PlotWorker(workerNum));
			workerList.at(workerNum)->moveToThread(threadList.at(i));
			connect(this, &PlotWindow::sendData, 
				workerList.at(workerNum), &PlotWorker::transferPlotData);
			connect(workerList.at(workerNum), &PlotWorker::dataTransferDone,
					this, &PlotWindow::countPlotsUpdated);
		}
		threadList.at(i)->start();
	}

	/* The first worker replots the whole QCustomPlot */
	connect(this, &PlotWindow::allSubplotsUpdated,
			workerList.at(0), &PlotWorker::replot);
}

void PlotWindow::initPlotGroup(void) {
	plot = new QCustomPlot(this);
	plot->plotLayout()->removeAt(0);
	plot->plotLayout()->expandTo(nrows, ncols);
	plot->plotLayout()->setRowSpacing(-10);
	plot->plotLayout()->setColumnSpacing(-10);

	QFont labelFont("Helvetica", 12, QFont::Light);
	int labelPadding = 3;
	QStringList channelLabels;
	channelLabels << QString("Photodiode");
	channelLabels << QString("Intracellular Vm");
	channelLabels << QString("Intracellular Im");
	channelLabels << QString("Extra");
	for (auto i = 4; i < (nrows * ncols); i++)
		channelLabels << QString::number(i);

	for (auto i = 0; i < nrows; i++) {
		for (auto j = 0; j < nrows; j++) {
			auto chan = i * ncols + j;
			/* Construct an axis in the grid */
			QPair<int, int> pos = settings.getChannelView().at(chan);
			QCPAxisRect *rect = new QCPAxisRect(plot);
			plot->plotLayout()->addElement(pos.first, pos.second, rect);

			/* Add a graph to it and format */
			QCPGraph *g = new QCPGraph(
					rect->axis(QCPAxis::atBottom), rect->axis(QCPAxis::atLeft));
			plot->addPlottable(g);
			subplotList.append(g);
			g->keyAxis()->setTicks(false);
			g->keyAxis()->setTickLabels(false);
			g->keyAxis()->grid()->setVisible(false);
			g->keyAxis()->setRange(0, H5Rec::SAMPLE_RATE *
					settings.getRefreshInterval() / 1000);
			g->keyAxis()->setLabel(channelLabels.at(chan));
			g->keyAxis()->setLabelFont(labelFont);
			g->keyAxis()->setLabelPadding(labelPadding);

			g->valueAxis()->setTicks(false);
			g->valueAxis()->setTickLabels(false);
			g->valueAxis()->grid()->setVisible(false);
			g->valueAxis()->setRange(
					-(settings.getDisplayScale() * NEG_DISPLAY_RANGE),
					settings.getDisplayScale() * POS_DISPLAY_RANGE);
		}
	}

	connect(plot, &QCustomPlot::mouseDoubleClick,
			this, &PlotWindow::createChannelInspector);
	connect(plot, &QCustomPlot::mousePress,
			this, &PlotWindow::handleChannelClick);

	layout = new QGridLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(plot);
	setLayout(layout);
}

void PlotWindow::plotData(H5Rec::Samples &s) {
	for (auto i = 0; i < nrows; i++) {
		for (auto j = 0; j < ncols; j++) {
			int channel = i * ncols + j;
			// Plotting thread must delete this!
			QVector<double> *data = new QVector<double>(s.n_rows);
			for (auto k = 0; k < s.n_rows; k++)
				data->replace(k, s(k, channel));
			emit sendData(sem, channel, channel, 
					subplotList.at(channel), data, 
					clickedPlots.contains(channel));
		}
	}
}

void PlotWindow::countPlotsUpdated(void) {
	numPlotsUpdated++;
	if (numPlotsUpdated < subplotList.size())
		return;
	numPlotsUpdated = 0;
	emit allSubplotsUpdated(sem, numThreads, plot);
}

void PlotWindow::toggleVisible() {
	this->setVisible(!this->isVisible());
}

void PlotWindow::clearAll(void) {
	sem->acquire(numThreads);
	for (auto &subplot : subplotList)
		subplot->clearData();
	plot->replot();
	sem->release(numThreads);
}

void PlotWindow::createChannelInspector(QMouseEvent *event) {
	/* Find the graph that was clicked */
	int channel = findSubplotClicked(event->pos());
	if (channel == -1)
		return;
	ChannelInspector *c = new ChannelInspector(plot,
			subplotList.at(channel), channel, this);
	c->show();
}

void PlotWindow::handleChannelClick(QMouseEvent *event) {
	if (event->button() != Qt::RightButton)
		return;
	int channel = findSubplotClicked(event->pos());
	if (channel == -1)
		return;
	if (clickedPlots.contains(channel))
		clickedPlots.remove(channel);
	else
		clickedPlots.insert(channel);
}

int PlotWindow::findSubplotClicked(QPoint pos) {
	QList<QCPAxisRect *> rects = plot->axisRects();
	int channel = -1;
	for (auto i = 0; i < rects.size(); i++) {
		if (rects.at(i)->outerRect().contains(pos)) {
			channel = i;
			break;
		}
	}
	return channel;
}

void PlotWindow::waitAll(void) {
	sem->acquire(numThreads);
}

void PlotWindow::forceReplot(void) {
	sem->acquire(numThreads);
	plot->replot();
	sem->release(numThreads);
}

