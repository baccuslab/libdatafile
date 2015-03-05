/* mainwindow.cpp
 * --------------
 * Implementation of the main GUI window for the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	this->setGeometry(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	this->setWindowTitle("Meaview: Channel view");
	initSettings();
	initCtrlWindow();
	initMenuBar();
	//initToolBar();
	initStatusBar();
	initPlotGroup();
}

MainWindow::~MainWindow() {
}

void MainWindow::initSettings() {
	settings.setSaveDir(DEFAULT_SAVE_DIR);
	settings.setSaveFilename(DEFAULT_SAVE_FILENAME);
	settings.setChannelView(DEFAULT_VIEW);
	settings.setExperimentLength(DEFAULT_EXPERIMENT_LENGTH);
	settings.setDisplayScale(DEFAULT_DISPLAY_SCALE);
	settings.setRefreshInterval(DISPLAY_REFRESH_INTERVAL);
	settings.setPlotColor(DEFAULT_PLOT_COLOR);
	settings.setAutoscale(false);
	settings.setOnlineAnalysisLength(DEFAULT_ONLINE_ANALYSIS_LENGTH);
	settings.setJump(AIB_BLOCK_SIZE);
	QPair<int, int> plotSize = CHANNEL_COL_ROW_MAP.value(
			settings.getChannelViewString());
	settings.setNumRows(plotSize.first);
	settings.setNumCols(plotSize.second);
}

void MainWindow::initCtrlWindow() {
	this->ctrlWindow = new CtrlWindow(this);
	this->ctrlWindow->show();
}

void MainWindow::initMenuBar() {
	this->menubar = new QMenuBar(0);

	/* File menu */
	this->fileMenu = new QMenu(tr("&File"));

	/* About menu item */
	//QMenu *aboutMenu = new QMenu("About Meaview");
	//this->menubar->addMenu(aboutMenu);

	/* New recording menu item */
	QAction *newRecordingAction = new QAction(tr("&New"), this->fileMenu);
	newRecordingAction->setShortcut(QKeySequence("Ctrl+N"));
	connect(newRecordingAction, SIGNAL(triggered()), this, SLOT(openNewRecording()));
	this->fileMenu->addAction(newRecordingAction);

	/* Load recording for replay */
	QAction *loadRecordingAction = new QAction(tr("&Open"), this->fileMenu);
	loadRecordingAction->setShortcut(QKeySequence("Ctrl+O"));
	connect(loadRecordingAction, SIGNAL(triggered()), this, SLOT(loadRecording()));
	this->fileMenu->addAction(loadRecordingAction);

	/* Windows menu */
	this->windowsMenu = new QMenu(tr("&Windows"));
	QAction *showMainWindow = new QAction(tr("&Channel view"), this->windowsMenu);
	showMainWindow->setShortcut(QKeySequence("Ctrl+0"));
	showMainWindow->setCheckable(true);
	showMainWindow->setChecked(true);
	connect(showMainWindow, SIGNAL(triggered()), this, SLOT(toggleVisible()));
	this->windowsMenu->addAction(showMainWindow);

	QAction *showControlsWindow = new QAction(tr("Control window"), this->windowsMenu);
	showControlsWindow->setShortcut(QKeySequence("Ctrl+1"));
	showControlsWindow->setCheckable(true);
	showControlsWindow->setChecked(true);
	connect(showControlsWindow, SIGNAL(triggered()), this->ctrlWindow, SLOT(toggleVisible()));
	this->windowsMenu->addAction(showControlsWindow);

	/* eventually same for online analysis and channel inspector */

	/* Add menus to bar and bar to MainWindow */
	this->menubar->addMenu(this->fileMenu);
	this->menubar->addMenu(this->windowsMenu);
	this->setMenuBar(this->menubar);
}


void MainWindow::openSettings() {
	SettingsWindow w(this);
	w.exec();
}

void MainWindow::initStatusBar() {
	this->statusBar = new QStatusBar();
	statusLabel = new QLabel("Ready");
	this->statusBar->addWidget(statusLabel);
	this->setStatusBar(this->statusBar);
}

void MainWindow::initPlotGroup() {
	//channelPlotGroup = new QWidget(this);
	//channelLayout = new QGridLayout();
	//channelPlots.resize(NUM_CHANNELS);
	//for (int c = 0; c < NUM_CHANNELS; c++) {
		//QPair<int, int> pos = this->settings.getChannelView().at(c);
		//channelPlots.at(c) = new ChannelPlot(c, pos);
		//channelLayout->addWidget(channelPlots.at(c), pos.first, pos.second, 1, 1);
		////connect(channelPlots.at(c), SIGNAL(mouseDoubleClick(QMouseEvent *)),
				////this, SLOT(openSingleChannel()));
	//}
	//channelPlotGroup->setLayout(channelLayout);
	//this->setCentralWidget(channelPlotGroup);
	this->channelPlot = new ChannelPlot(this->settings.getNumRows(),
			this->settings.getNumCols(), this);
	this->setCentralWidget(this->channelPlot);
	//this->p = new QCustomPlot(this);
	//QCPLayoutGrid *l = this->p->plotLayout();
	//l->removeAt(0);
	//l->expandTo(this->settings.getNumRows(), this->settings.getNumCols());
	//for (int c = 0; c < NUM_CHANNELS; c++) {
		//QPair<int, int> pos = this->settings.getChannelView().at(c);
		//QCPAxisRect *r = new QCPAxisRect(this->p);
		//l->addElement(pos.first, pos.second, r);
		//QCPGraph *g = new QCPGraph(r->axis(QCPAxis::atBottom), r->axis(QCPAxis::atLeft));
		//p->addPlottable(g);
		//g->keyAxis()->setTicks(false);
		//g->keyAxis()->setTickLabels(false);
		//g->keyAxis()->grid()->setVisible(false);
		//g->keyAxis()->setRange(0, 20000);
		//g->valueAxis()->setTicks(false);
		//g->valueAxis()->setTickLabels(false);
		//g->valueAxis()->grid()->setVisible(false);
		//g->valueAxis()->setRange(-NEG_DISPLAY_RANGE, POS_DISPLAY_RANGE);
	//}

	//this->setCentralWidget(p);
	//[> X-values for all plots <]
	//QVector<double> tmpX(SAMPLE_RATE * (DISPLAY_REFRESH_INTERVAL / 1000));
	//double start = 0.0;
	//iota(tmpX.begin(), tmpX.end(), start);
	//this->PLOT_X = tmpX;
}

void MainWindow::openNewRecording() {
	NewRecordingWindow w(this);
	int ret;

	while (true) {
		ret = w.exec();
		if (ret == QDialog::Rejected)
			return;
		if ((ret = w.validateChoices()) != 0)
			w.close();
		else
			break;
	}

	/* Validated file name */
	QString filename = w.getFullFilename();
	qDebug() << "File: " << filename << endl;

	/* Make a file */
	//this->recording = new LiveRecording(filename, w.getTime());
}

void MainWindow::loadRecording() {
	this->statusLabel->setText("Loading recording");
	QString filename = QFileDialog::getOpenFileName(
			this, tr("Load recording"),
			DEFAULT_SAVE_DIR, tr("Recordings (*.bin)"));
	if (filename.isNull())
		return;
	
	/* Open the recording */
	this->recording = new PlaybackRecording(filename);
	this->initPlaybackRecording();
	this->statusLabel->setText("Ready");
}

void MainWindow::initPlaybackRecording() {
	this->ctrlWindow->startPauseButton->setEnabled(true);
	this->ctrlWindow->timeLine->setText("0");
	this->ctrlWindow->timeLine->setReadOnly(false); // Until play back started
	connect(this->ctrlWindow->startPauseButton, SIGNAL(clicked()), this, SLOT(togglePlayback()));
	this->playbackTimer = new QTimer();
	this->playbackTimer->setInterval(this->settings.getRefreshInterval());
	//connect(this->playbackTimer, SIGNAL(timeout()), this, SLOT(plotNextDataBlock()));
	connect(this->playbackTimer, SIGNAL(timeout()), this, SLOT(plot()));
}

void MainWindow::setScale(int s) {
	this->settings.setDisplayScale(DISPLAY_SCALES[s]);
	this->scaleBox->setCurrentIndex(s);
}

void MainWindow::togglePlayback() {
	if (this->isPlaying) {
		this->playbackTimer->stop();
		this->ctrlWindow->startPauseButton->setText("Start");
		this->ctrlWindow->startPauseButton->setStyleSheet("QPushButton {color : black}");
	} else {
		this->playbackTimer->start();
		this->ctrlWindow->startPauseButton->setText("Stop");
		this->ctrlWindow->startPauseButton->setStyleSheet("QPushButton {color : rgb(178, 51, 51)}");
	}
	this->isPlaying = !this->isPlaying;
}

void MainWindow::plot() {
	//QtConcurrent::run(this, &MainWindow::plotNextDataBlock);
	QVector<QVector<int16_t> > data = this->recording->getNextDataBlock();
	QtConcurrent::run(this->channelPlot, &ChannelPlot::plotData, data);
}

//void MainWindow::plotNextDataBlock() {

	//[> Trying concurrent move of data. Doesn't really seem to work, though.
	 //* Doesn't crash, but the plots are only updated when we click on them.
	 //* The finished() signal might not really be connected to the replot()
	 //* slot, could be because these are on the stack?
	 //*
	//QFutureWatcher<void> watcher;
	//connect(&watcher, SIGNAL(finished()), this->p, SLOT(replot()));
	//QFuture<void> future = QtConcurrent::run(this, &MainWindow::transferDataToPlots, data);
	//qDebug() << "Load finished, future dispatched: " << QTime::currentTime();
	//*/

	//QPen pen = this->settings.getPlotPen();
	//double scale = this->settings.getDisplayScale();
	//qDebug() << "Load start: " << QTime::currentTime();
	//QVector<QVector<int16_t> > data = this->recording->getNextDataBlock();
	//qDebug() << "Load end: " << QTime::currentTime();
	//qDebug() << "Set data start: " << QTime::currentTime();
	//for (auto i = 0; i < NUM_CHANNELS; i++) {
		//QVector<double> tmp(AIB_BLOCK_SIZE, 0);
		//for (auto j = 0; j < AIB_BLOCK_SIZE; j++)
			//tmp[j] = data.at(i).at(j);
		//QCPGraph *g = this->p->graph(i);
		//g->setData(PLOT_X, tmp);
		//if (this->ctrlWindow->autoscale) {
			//g->valueAxis()->rescale();
		//} else {
			//g->valueAxis()->setRange(-(scale * NEG_DISPLAY_RANGE), (scale * POS_DISPLAY_RANGE));
		//}
		//g->setPen(pen);
	//}
	//qDebug() << "Set data end: " << QTime::currentTime();
	//qDebug() << "Replot start: " << QTime::currentTime();
	//this->p->replot();
	//qDebug() << "Replot end: " << QTime::currentTime() << endl;
//}


/* Naive asynchronous function for transferring data to plots
 * in a separate thread.
 *
void MainWindow::transferDataToPlots(QVector<QVector<int16_t> > data) {
	QVector<double> tmp(AIB_BLOCK_SIZE, 0);
	QPen pen = this->settings.getPlotPen();
	double scale = this->settings.getDisplayScale();
	for (auto i = 0; i < NUM_CHANNELS; i++) {
		for (auto j = 0; j < AIB_BLOCK_SIZE; j++)
			tmp[j] = data.at(i).at(j);
		QCPGraph *g = this->p->graph(i);
		g->setData(PLOT_X, tmp);
		if (this->ctrlWindow->autoscale) {
			g->valueAxis()->rescale();
		} else {
			g->valueAxis()->setRange(-(scale * NEG_DISPLAY_RANGE), (scale * POS_DISPLAY_RANGE));
		}
		g->setPen(pen);
	}
	qDebug() << "Finished async run" << QTime::currentTime() << endl;
}
*/

void MainWindow::setAutoscale(int state) {
	bool set = state == Qt::Checked;
	autoscale = set;
	this->autoscaleCheckBox->setChecked(set);
	this->scaleBox->setEnabled(!set);
	this->settings.setAutoscale(set);
}

void MainWindow::toggleVisible() {
	this->setVisible(!this->isVisible());
}
