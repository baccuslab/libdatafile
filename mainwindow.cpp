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
	this->channelPlot = new ChannelPlot(this->settings.getNumRows(),
			this->settings.getNumCols(), this);
	this->setCentralWidget(this->channelPlot);
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
	connect(this->playbackTimer, SIGNAL(timeout()), this, SLOT(plotNextDataBlock()));
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

void MainWindow::plotNextDataBlock() {
	QVector<QVector<int16_t> > data = this->recording->getNextDataBlock();
	QtConcurrent::run(this->channelPlot, &ChannelPlot::plotData, data);
}

void MainWindow::toggleVisible() {
	this->setVisible(!this->isVisible());
}
