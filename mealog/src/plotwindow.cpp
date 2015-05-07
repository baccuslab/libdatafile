/* plotwindow.cpp
 * --------------
 * Implementation of the data plotting window for the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <QPair>
#include "plotwindow.h"

PlotWindow::PlotWindow(int numRows, int numCols, QWidget *parent) : 
		QWidget(parent, Qt::Window) {
	nrows = numRows;
	ncols = numCols;
	sem = new QSemaphore(NUM_THREADS);
	setGeometry(0, 0, PLOT_WINDOW_WIDTH, PLOT_WINDOW_HEIGHT);
	setWindowTitle("Meaview: Channel view");
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
	int nworkersPerThread = H5Rec::NUM_CHANNELS / NUM_THREADS;
	for (auto i = 0; i < NUM_THREADS; i++) {
		threadList.append(new QThread());
		for (auto j = 0; j < nworkersPerThread; j++) {
			int workerNum = i * nworkersPerThread + j;
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
			g->valueAxis()->setTicks(false);
			g->valueAxis()->setTickLabels(false);
			g->valueAxis()->grid()->setVisible(false);
			g->valueAxis()->setRange(
					-(settings.getDisplayScale() * NEG_DISPLAY_RANGE),
					settings.getDisplayScale() * POS_DISPLAY_RANGE);
		}
	}
	
	layout = new QGridLayout(this);
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
					subplotList.at(channel), data);
		}
	}
}

void PlotWindow::countPlotsUpdated(void) {
	numPlotsUpdated++;
	if (numPlotsUpdated < subplotList.size())
		return;
	numPlotsUpdated = 0;
	emit allSubplotsUpdated(sem, NUM_THREADS, plot);
}

void PlotWindow::toggleVisible() {
	this->setVisible(!this->isVisible());
}

void PlotWindow::clearAll(void) {
	sem->acquire(NUM_THREADS);
	for (auto &subplot : subplotList)
		subplot->clearData();
	plot->replot();
	sem->release(NUM_THREADS);
}

