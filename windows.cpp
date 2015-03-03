/* windows.cpp
 * Implementation of various windows used in the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

/* C++ includes */

#if QT_VERSION < 0x050000
/* Qt includes */
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QDebug>
#include <QtConcurrent>
#endif

/* meaview includes */
#include "windows.h"

/***************************************************
 ***************** MainWindow **********************
 ***************************************************/
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
	settings->setValue("savedir", QVariant(QString::fromStdString(DEFAULT_SAVE_DIR)));
	settings->setValue("filename", QVariant(QString::fromStdString(DEFAULT_SAVE_FILENAME)));
	settings->setValue("view", QVariant(QString::fromStdString(DEFAULT_VIEW)));
	settings->setValue("time", QVariant(DEFAULT_RECORD_LENGTH));
	settings->setValue("scale", QVariant(DEFAULT_DISPLAY_SCALE));
	settings->setValue("refresh", QVariant(DISPLAY_REFRESH_INTERVAL));
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
	//connect(this->startButton, SIGNAL(triggered()), this, SLOT(startPlayback()));
	toolbar->addWidget(this->startButton);

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
		pos_t pos = CHANNEL_VIEWS.at(DEFAULT_VIEW).at(c);
		channelPlots.at(c) = new ChannelPlot(c, pos);
		channelLayout->addWidget(channelPlots.at(c), pos.row, pos.col, 1, 1);
		//connect(channelPlots.at(c), SIGNAL(mouseDoubleClick(QMouseEvent *)),
				//this, SLOT(openSingleChannel()));
	}
	channelPlotGroup->setLayout(channelLayout);
	this->setCentralWidget(channelPlotGroup);
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
			QString::fromStdString(DEFAULT_SAVE_DIR), tr("Recordings (*.bin)"));
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
	//QComboBox *sender = dynamic_cast<QComboBox *>(QObject::sender());
	//this->settings->setValue("scale", sender->currentData());
	this->settings->setValue("scale", QVariant(s));
	/* Need to make this more general. Slot may be usefully called
	 * from many senders, not just the ComboBox
	 */
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
	QVector<QVector<int16_t> > data = this->recording->getNextDataBlock();
	for (auto i = 0; i < NUM_CHANNELS; i++) {
		QVector<double> tmp(AIB_BLOCK_SIZE, 0);
		for (auto j = 0; j < AIB_BLOCK_SIZE; j++)
			tmp[j] = data.at(i).at(j);
		//qDebug() << "Channel: " << i << " element 7: " << data.at(i).at(7);
		//qDebug() << "Plot: " << i << " element 7: " << tmp.at(7);
		this->channelPlots.at(i)->graph(0)->setData(x, tmp);
		this->channelPlots.at(i)->rescaleAxes();
		this->channelPlots.at(i)->replot();
	}
	qDebug() << "Done plotting block";
}


/***************************************************
 ************** NewRecordingWindow *****************
 ***************************************************/
NewRecordingWindow::NewRecordingWindow(QWidget *parent) : QDialog(parent) {
	/* Default choices */
	settings = new QSettings();

	/* Selection for plot arrangement */
	viewGroup = new QGroupBox("Channel view");
	viewLayout = new QVBoxLayout();
	viewBox = new QComboBox(this);
	for (auto &view : VIEW_LABELS)
		viewBox->addItem(QString::fromStdString(view));
	viewBox->setCurrentIndex(viewBox->findText(QString::fromStdString(DEFAULT_VIEW)));
	connect(viewBox, SIGNAL(activated()), this, SLOT(setView()));
	viewLayout->addWidget(viewBox);
	viewGroup->setLayout(viewLayout);

	/* Select save directory */
	saveGroup = new QGroupBox("Save directory");
	saveLine = new QLineEdit(QString::fromStdString(DEFAULT_SAVE_DIR));
	saveLine->setReadOnly(true);
	browseButton = new QPushButton("Browse");
	connect(browseButton, SIGNAL(clicked()), this, SLOT(chooseDirectory()));
	saveLayout = new QGridLayout();
	saveLayout->addWidget(saveLine, 0, 0);
	saveLayout->addWidget(browseButton, 0, 1);
	saveGroup->setLayout(saveLayout);

	/* Select filename */
	fileGroup = new QGroupBox(tr("&Filename"));
	fileLine = new QLineEdit(QString::fromStdString(DEFAULT_SAVE_FILENAME));
	fileValidator = new QRegExpValidator(QRegExp(QString::fromStdString("(\\w+[-_]*)+")));
	fileLine->setValidator(fileValidator);
	fileLayout = new QVBoxLayout();
	fileLayout->addWidget(fileLine);
	fileGroup->setLayout(fileLayout);

	/* Pick a length of the recording */
	timeGroup = new QGroupBox("Length of recording");
	timeValidator = new QIntValidator(0, MAX_RECORD_LENGTH);
	timeLine = new QLineEdit(QString::number(DEFAULT_RECORD_LENGTH));
	timeLine->setValidator(timeValidator);
	timeLayout = new QVBoxLayout();
	timeLayout->addWidget(timeLine);
	timeGroup->setLayout(timeLayout);

	/* OK/cancel buttons */
	buttonGroup = new QGroupBox();
	buttonGroup->setFlat(true);
	okButton = new QPushButton("OK");
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	okButton->setDefault(true);
	cancelButton = new QPushButton("Cancel");
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	buttonLayout = new QHBoxLayout();
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);
	buttonGroup->setLayout(buttonLayout);

	/* Layout */
	layout = new QGridLayout();
	layout->addWidget(buttonGroup, 2, 0);
	layout->addWidget(viewGroup, 0, 0, 1, 2);
	layout->addWidget(fileGroup, 0, 2, 1, 2);
	layout->addWidget(timeGroup, 0, 4, 1, 1);
	layout->addWidget(saveGroup, 1, 0, 1, 5);

	this->setLayout(layout);
	this->setWindowTitle("Create new recording");

}

NewRecordingWindow::~NewRecordingWindow() {
}

QString NewRecordingWindow::getSaveDir() {
	return (this->settings->value("savedir")).toString();
}

QString NewRecordingWindow::getSaveFilename() {
	return (this->settings->value("filename")).toString();
}

QString NewRecordingWindow::getView() {
	return (this->settings->value("view")).toString();
}

uint NewRecordingWindow::getTime() {
	return this->settings->value("time").toUInt();
}

QString NewRecordingWindow::getFullFilename() {
	QString s = this->getSaveDir();
	if (!s.endsWith("/"))
		s.append("/");
	s.append(this->getSaveFilename());
	return s.append(QString::fromStdString(SAVE_FILE_EXTENSION));
}

void NewRecordingWindow::setView() {
	//QAction *sender = dynamic_cast<QAction *>(QObject::sender());
	//this->viewButton->setText(sender->text());
	QComboBox *sender = dynamic_cast<QComboBox *>(QObject::sender());
	this->settings->value("view", sender->currentText());
}

int NewRecordingWindow::validateChoices() {
	this->settings->setValue("savedir", QVariant(this->saveLine->text()));
	this->settings->setValue("filename", QVariant(this->fileLine->text()));
	this->settings->setValue("view", QVariant(this->viewBox->currentText()));
	this->settings->setValue("time", QVariant(this->timeLine->text()));
	QFileInfo finfo(this->saveLine->text());
	if (!finfo.permission(QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
		QMessageBox msg;
		msg.setText("Permissions error");
		msg.setInformativeText(QString(
				"The current user does not have permissions for"
				" the requested save directory:\n%1").arg(
				this->saveLine->text()));
		msg.setStandardButtons(QMessageBox::Ok);
		msg.exec();
		return -1;
	}
	return 0;
}

void NewRecordingWindow::chooseDirectory() {
	QFileDialog dialog(this, "Choose save directory", QString::fromStdString(DEFAULT_SAVE_DIR));
	dialog.setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dialog.exec() == QDialog::Rejected)
		return;
	QString path = dialog.directory().absolutePath();
	this->settings->setValue("savedir", QVariant(path));
	this->saveLine->setText(path);
}

