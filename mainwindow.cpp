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
	initMenuBar();
	initToolBar();
	initStatusBar();
	initPlotGroup();
}

MainWindow::~MainWindow() {
}

void MainWindow::initSettings() {
	QCoreApplication::setOrganizationName("baccuslab");
	QCoreApplication::setApplicationName("meaview");
	settings = new QSettings();
	settings->setValue("savedir", QVariant(DEFAULT_SAVE_DIR));
	settings->setValue("filename", QVariant(DEFAULT_SAVE_FILENAME));
	settings->setValue("view", QVariant(DEFAULT_VIEW));
	settings->setValue("time", QVariant(DEFAULT_RECORD_LENGTH));
	settings->setValue("scale", QVariant(DEFAULT_DISPLAY_SCALE));
	settings->setValue("refresh", QVariant(DISPLAY_REFRESH_INTERVAL));
	settings->setValue("pen", QVariant(PEN_MAP[DEFAULT_PLOT_COLOR]));
	settings->setValue("pen-label", QVariant(DEFAULT_PLOT_COLOR));
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

	/* Add menus to bar and bar to MainWindow */
	this->menubar->addMenu(this->fileMenu);
	this->setMenuBar(this->menubar);
}

void MainWindow::initToolBar() {
	toolbar = new QToolBar();

	/* Group showing current time and total time */
	timeGroup = new QGroupBox();
	timeLabel = new QLabel("Time:");
	timeLine = new QLineEdit("");
	timeLine->setMaxLength(5);
	timeValidator = new QIntValidator(0, MAX_RECORD_LENGTH);
	timeLine->setValidator(timeValidator);
	timeLine->setReadOnly(true);
	timeLayout = new QHBoxLayout();
	timeLayout->addWidget(timeLabel);
	timeLayout->addWidget(timeLine);
	timeGroup->setLayout(timeLayout);
	toolbar->addWidget(timeGroup);
	
	toolbar->addSeparator();

	/* Button to start/stop recording */
	startButton = new QPushButton("Start");
	startButton->setEnabled(false);
	startButton->setShortcut(QKeySequence("Ctrl+Enter"));
	//connect(this->startButton, SIGNAL(triggered()), this, SLOT(startPlayback()));
	toolbar->addWidget(this->startButton);

	settingsButton = new QPushButton("Settings");
	connect(this->settingsButton, SIGNAL(clicked()), this, SLOT(openSettings()));
	toolbar->addWidget(settingsButton);

	toolbar->addSeparator();

	scaleGroup = new QGroupBox();
	scaleLayout = new QHBoxLayout();
	scaleLabel = new QLabel("Scale:");
	scaleBox = new QComboBox();
	for (auto &s: DISPLAY_SCALES)
		scaleBox->addItem(QString::number(s), QVariant(s));
	scaleBox->setCurrentIndex(scaleBox->findData(QVariant(DEFAULT_DISPLAY_SCALE)));
	connect(scaleBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setScale(int)));
	scaleLayout->addWidget(scaleLabel);
	scaleLayout->addWidget(scaleBox);
	scaleGroup->setLayout(scaleLayout);
	toolbar->addWidget(scaleGroup);

	addToolBar(toolbar);
}

void MainWindow::openSettings() {
	SettingsWindow w;
	w.exec();
}

void MainWindow::initStatusBar() {
	this->statusBar = new QStatusBar();
	QLabel *statusLabel = new QLabel("Ready");
	this->statusBar->addWidget(statusLabel);
	this->setStatusBar(this->statusBar);
}

void MainWindow::initPlotGroup() {
	channelPlotGroup = new QWidget(this);
	channelLayout = new QGridLayout();
	channelPlots.resize(NUM_CHANNELS);
	for (int c = 0; c < NUM_CHANNELS; c++) {
		QPair<int, int> pos = CHANNEL_VIEWS.value(DEFAULT_VIEW).at(c);
		channelPlots.at(c) = new ChannelPlot(c, pos);
		channelLayout->addWidget(channelPlots.at(c), pos.first, pos.second, 1, 1);
		//connect(channelPlots.at(c), SIGNAL(mouseDoubleClick(QMouseEvent *)),
				//this, SLOT(openSingleChannel()));
	}
	channelPlotGroup->setLayout(channelLayout);
	this->setCentralWidget(channelPlotGroup);

	/* X-values for all plots */
	QVector<double> tmpX(SAMPLE_RATE * (DISPLAY_REFRESH_INTERVAL / 1000));
	double start = 0.0;
	iota(tmpX.begin(), tmpX.end(), start);
	const QVector<double> PLOT_X(tmpX);
}

void MainWindow::openNewRecording() {
	NewRecordingWindow *w = new NewRecordingWindow(this);
	int ret;

	while (true) {
		ret = w->exec();
		if (ret == QDialog::Rejected)
			return;
		if ((ret = w->validateChoices()) != 0)
			w->close();
		else
			break;
	}

	/* Validated file name */
	QString filename = w->getFullFilename();
	qDebug() << "File: " << filename << endl;

	/* Make a file */
	//this->recording = new LiveRecording(filename, w->getTime());
}

void MainWindow::loadRecording() {
	QString filename = QFileDialog::getOpenFileName(
			this, tr("Load recording"),
			DEFAULT_SAVE_DIR, tr("Recordings (*.bin)"));
	if (filename.isNull())
		return;
	
	/* Open the recording */
	this->recording = new PlaybackRecording(filename);
	qDebug() << "Block size: " << this->recording->getFile().getBlockSize();
	this->initPlaybackRecording();
}

void MainWindow::initPlaybackRecording() {
	this->startButton->setEnabled(true);
	this->timeLine->setText("0");
	this->timeLine->setReadOnly(false); // Until play back started
	connect(this->startButton, SIGNAL(clicked()), this, SLOT(togglePlayback()));
	this->playbackTimer = new QTimer();
	this->playbackTimer->setInterval(this->settings->value("refresh").toInt());
	connect(this->playbackTimer, SIGNAL(timeout()), this, SLOT(plotNextDataBlock()));
}

void MainWindow::setScale(int s) {
	this->settings->setValue("scale", QVariant(DISPLAY_SCALES[s]));
}

void MainWindow::togglePlayback() {
	qDebug() << "Playback toggled";
	if (this->isPlaying) {
		this->playbackTimer->stop();
		this->startButton->setText("Start");
		this->startButton->setStyleSheet("QPushButton {color : black}");
	} else {
		this->playbackTimer->start();
		this->startButton->setText("Stop");
		this->startButton->setStyleSheet("QPushButton {color : rgb(178, 51, 51)}");
	}
	this->isPlaying = !this->isPlaying;
}

void MainWindow::plotNextDataBlock() {
	qDebug() << "Getting next data block";
	//QFuture<QVector<QVector<int16_t> > > dataFuture = QtConcurrent::run(
			//this->recording, &PlaybackRecording::getNextDataBlock);
	QVector<double> x(this->recording->getFile().getBlockSize(), 0);
	for (auto i = 0; i < AIB_BLOCK_SIZE; i++)
		x[i] = i;
	double scale = this->settings->value("scale").toDouble();
	qDebug() << "Scale :" << scale << "Range: " << scale * DISPLAY_RANGE;
	QVector<QVector<int16_t> > data = this->recording->getNextDataBlock();
	QPen pen(PEN_MAP[this->settings->value("pen-label").toString()]);
	for (auto i = 0; i < NUM_CHANNELS; i++) {
		QVector<double> tmp(AIB_BLOCK_SIZE, 0);
		double min = 1000000;
		for (auto j = 0; j < AIB_BLOCK_SIZE; j++) {
			tmp[j] = data.at(i).at(j);
			if (tmp[j] < min)
				min = tmp[j];
		}
		this->channelPlots.at(i)->graph(0)->setData(x, tmp);
		this->channelPlots.at(i)->yAxis->setRange(-(scale * DISPLAY_RANGE),
				(scale * DISPLAY_RANGE));
		this->channelPlots.at(i)->graph(0)->setPen(pen);
		this->channelPlots.at(i)->replot();
	}
	qDebug() << "Done plotting block";
}
