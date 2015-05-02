/* mealogwindow.cpp
 * This file implements the mealog application's main window.
 * This application can be used to set parameters, start and stop 
 * recordings.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <string>

#include <QApplication>
#include <QVariant>
#include <QDebug>
#include <QHostAddress>
#include <QMessageBox>
#include <QTime>
#include <QDate>
#include <QDataStream>
#include <QFileDialog>
#include <QErrorMessage>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>

#include <cstdio>

#include <armadillo>

#include "mealogwindow.h"

MealogWindow::MealogWindow(QWidget *parent) : QMainWindow(parent) {
	initSettings();
	initGui();
	initMenuBar();
	initPlotWindow();
	//initServer();
	initSignals();
}

MealogWindow::~MealogWindow() {
}

void MealogWindow::initSettings() {
	settings.setSaveDir(STARTING_SAVE_DIR);
	settings.setChannelView(DEFAULT_VIEW);
	settings.setExperimentLength(DEFAULT_EXPERIMENT_LENGTH);
	settings.setDisplayScale(DEFAULT_DISPLAY_SCALE);
	settings.setRefreshInterval(DISPLAY_REFRESH_INTERVAL);
	settings.setPlotColor(DEFAULT_PLOT_COLOR);
	settings.setAutoscale(false);
	settings.setOnlineAnalysisLength(DEFAULT_ONLINE_ANALYSIS_LENGTH);
	settings.setJump(DISPLAY_REFRESH_INTERVAL);
	QPair<int, int> plotSize = CHANNEL_COL_ROW_MAP.value(
			settings.getChannelViewString());
	settings.setNumRows(plotSize.first);
	settings.setNumCols(plotSize.second);
	settings.setAutoMean(false);
}

void MealogWindow::initGui(void) {
	setWindowTitle("Mealog");
	setGeometry(0, 0, Mealog::WINDOW_WIDTH, Mealog::WINDOW_HEIGHT);
	mainLayout = new QGridLayout();

	/* Initialize top portion, with new/load buttons and path info */
	fileLabel = new QLabel("File:");
	fileLine = new QLineEdit(Mealog::DEFAULT_SAVE_FILE.fileName());
	fileLine->setToolTip("Name of file to which data is saved");
	fileValidator = new QRegExpValidator(QRegExp("(\\w+[-_]*)+"));
	fileLine->setValidator(fileValidator);
	choosePathButton = new QPushButton("Path");
	choosePathButton->setToolTip("Choose save directory");

	/* Initialize playback control group */
	playbackGroup = new QGroupBox("Controls");
	playbackLayout = new QGridLayout();
	timeLabel = new QLabel("Time:");
	timeLine = new QLineEdit();
	timeLine->setReadOnly(true);
	totalTimeLine = new QLineEdit(QString::number(Mealog::DEFAULT_EXPERIMENT_LENGTH));
	totalTimeLine->setToolTip("Length of the experiment (seconds)");
	totalTimeValidator = new QIntValidator(1, Mealog::MAX_EXPERIMENT_LENGTH);
	totalTimeLine->setValidator(totalTimeValidator);
	playButton = new QPushButton("Play");
	playButton->setToolTip("Play or pause recording. Does not affect saving of data");
	stopButton = new QPushButton("Stop");
	stopButton->setToolTip("Stop recording, including the saving of data");
	skipBackButton = new QPushButton("Back");
	skipBackButton->setToolTip("Skip backwards, without affecting saving");
	skipForwardButton = new QPushButton("Forward");
	skipForwardButton->setToolTip("Skip forwards, without affecting saving");
	skipToBeginningButton = new QPushButton("Start");
	skipToBeginningButton->setToolTip("Jump back to the start of the recording."\
			" Does not affect saving.");
	skipToEndButton = new QPushButton("End");
	skipToEndButton->setToolTip("Jump to end of recording or most recent data");
	setPlaybackButtonsEnabled(false);
	playbackLayout->addWidget(fileLabel, 0, 0);
	playbackLayout->addWidget(fileLine, 0, 1);
	playbackLayout->addWidget(choosePathButton, 0, 2);
	playbackLayout->addWidget(timeLabel, 1, 0);
	playbackLayout->addWidget(timeLine, 1, 1);
	playbackLayout->addWidget(totalTimeLine, 1, 2);
	playbackLayout->addWidget(skipBackButton, 2, 0);
	playbackLayout->addWidget(playButton, 2, 1);
	playbackLayout->addWidget(skipForwardButton, 2, 2);
	playbackLayout->addWidget(skipToBeginningButton, 3, 0);
	playbackLayout->addWidget(stopButton, 3, 1);
	playbackLayout->addWidget(skipToEndButton, 3, 2);
	playbackGroup->setLayout(playbackLayout);
	mainLayout->addWidget(playbackGroup, 0, 0, 1, 2);

	/* Initialize NI-DAQ group */
	nidaqGroup = new QGroupBox("NI-DAQ");
	nidaqLayout = new QGridLayout();
	connectToNidaqButton = new QPushButton("Connect");
	connectToNidaqButton->setToolTip("Connect to NI-DAQ server to initialize the recording");
	nidaqHostLabel = new QLabel("Host:");
	nidaqHostLabel->setAlignment(Qt::AlignRight);
	nidaqHost = new QLineEdit(Mealog::DEFAULT_NIDAQ_HOST);
	nidaqHost->setToolTip("IP address of the computer running the NI-DAQ server");
	nidaqValidator = new QRegExpValidator(
			QRegExp("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}"\
				"([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])"));
	nidaqHost->setValidator(nidaqValidator);
	nidaqStatusLabel = new QLabel("Status:");
	nidaqStatusLabel->setAlignment(Qt::AlignRight);
	nidaqStatus = new QLineEdit("Not connected");
	nidaqStatus->setReadOnly(true);
	nidaqStatus->setToolTip("Status of connection with NIDAQ");
	nidaqLayout->addWidget(nidaqHostLabel, 0, 0);
	nidaqLayout->addWidget(nidaqHost, 0, 1, 1, 2);
	nidaqLayout->addWidget(connectToNidaqButton, 0, 3);
	nidaqLayout->addWidget(nidaqStatusLabel, 1, 0);
	nidaqLayout->addWidget(nidaqStatus, 1, 1, 1, 3);
	nidaqGroup->setLayout(nidaqLayout);
	mainLayout->addWidget(nidaqGroup, 1, 0, 1, 2);

	/* Initialize group with parameters of the recording */
	recordingGroup = new QGroupBox("Recording parameters");
	recordingLayout = new QGridLayout();
	adcRangeLabel = new QLabel("ADC range:");
	adcRangeBox = new QComboBox();
	for (auto &each : Mealog::ADC_RANGES)
		adcRangeBox->addItem(QString::number(each), QVariant(each));
	adcRangeBox->setToolTip("Set the voltage range of the NI-DAQ card");
	adcRangeBox->setCurrentIndex(
			Mealog::ADC_RANGES.indexOf(Mealog::DEFAULT_ADC_RANGE));
	triggerBox = new QComboBox(this);
	triggerBox->addItems(Mealog::TRIGGERS);
	triggerLabel = new QLabel("Trigger:");
	triggerBox->setToolTip("Set the triggering mechanism for starting the experiment");
	recordingLayout->addWidget(adcRangeLabel, 0, 0);
	recordingLayout->addWidget(adcRangeBox, 0, 1);
	recordingLayout->addWidget(triggerLabel, 1, 0);
	recordingLayout->addWidget(triggerBox, 1, 1);
	recordingGroup->setLayout(recordingLayout);
	mainLayout->addWidget(recordingGroup, 1, 2);

	/* Initialize group with parameters of the data display window */
	displayGroup = new QGroupBox("Display");
	displayLayout = new QGridLayout();
	viewLabel = new QLabel("View:");
	viewBox = new QComboBox();
	viewBox->setToolTip("Set arrangement of subplots to match array");
	for (auto &view : CHANNEL_VIEW_STRINGS)
		viewBox->addItem(view);
	viewBox->setCurrentIndex(
			viewBox->findText(settings.getChannelViewString()));
	colorLabel = new QLabel("Color:");
	colorBox = new QComboBox();
	colorBox->setToolTip("Set color of data plots");
	for (auto &color : PLOT_COLOR_STRINGS)
		colorBox->addItem(color);
	colorBox->setCurrentIndex(
			colorBox->findText(settings.getPlotColorString()));
	scaleLabel = new QLabel("Scale:");
	scaleBox = new QComboBox();
	scaleBox->setToolTip("Change y-axis scaling, larger means larger axis extent");
	for (auto &scale : DISPLAY_SCALES)
		scaleBox->addItem(QString::number(scale));
	scaleBox->setCurrentIndex(
			settings.getDisplayScales().indexOf(settings.getDisplayScale()));
	autoscaleLabel = new QLabel("Autoscale:");
	autoscaleBox = new QCheckBox();
	autoscaleBox->setToolTip("If checked, each subplot will scale to fit its data");
	autoscaleBox->setTristate(false);
	autoscaleBox->setChecked(false);
	automeanLabel = new QLabel("Automean:");
	automeanBox = new QCheckBox();
	automeanBox->setToolTip("If checked, mean-subtract each plot individually");
	automeanBox->setTristate(false);
	automeanBox->setChecked(false);
	displayLayout->addWidget(viewLabel, 0, 0);
	displayLayout->addWidget(viewBox, 0, 1);
	displayLayout->addWidget(colorLabel, 1, 0);
	displayLayout->addWidget(colorBox, 1, 1);
	displayLayout->addWidget(scaleLabel, 2, 0);
	displayLayout->addWidget(scaleBox, 2, 1);
	displayLayout->addWidget(autoscaleLabel, 3, 0);
	displayLayout->addWidget(autoscaleBox, 3, 1);
	displayLayout->addWidget(automeanLabel, 4, 0);
	displayLayout->addWidget(automeanBox, 4, 1);
	displayGroup->setLayout(displayLayout);
	mainLayout->addWidget(displayGroup, 0, 2);

	statusBar = new QStatusBar(this);
	statusBar->showMessage("Ready");
	this->setStatusBar(statusBar);
	this->setCentralWidget(new QWidget(this));
	this->centralWidget()->setLayout(mainLayout);
}

void MealogWindow::setPlaybackButtonsEnabled(bool enabled) {
	skipBackButton->setEnabled(enabled);
	playButton->setEnabled(enabled);
	skipForwardButton->setEnabled(enabled);
	skipToBeginningButton->setEnabled(enabled);
	stopButton->setEnabled(enabled);
	skipToEndButton->setEnabled(enabled);
}

void MealogWindow::initMenuBar(void) {
	menubar = new QMenuBar(0);

	/* About mealog */
	//aboutAction = new QAction(tr("about.*"), menubar);
	//aboutAction->setMenuRole(QAction::AboutRole);

	/* File menu */
	fileMenu = new QMenu(tr("&File"));

	/* New recording menu item */
	newRecordingAction = new QAction(tr("&New"), fileMenu);
	newRecordingAction->setShortcut(QKeySequence("Ctrl+N"));
	connect(newRecordingAction, SIGNAL(triggered()), 
			this, SLOT(createNewRecording()));
	fileMenu->addAction(newRecordingAction);

	/* Load recording for replay */
	loadRecordingAction = new QAction(tr("&Open"), fileMenu);
	loadRecordingAction->setShortcut(QKeySequence("Ctrl+O"));
	connect(loadRecordingAction, SIGNAL(triggered()), 
			this, SLOT(loadRecording()));
	fileMenu->addAction(loadRecordingAction);

	/* Close recording */
	closeRecordingAction = new QAction(tr("&Close"), fileMenu);
	closeRecordingAction->setShortcut(QKeySequence("Ctrl+C"));
	closeRecordingAction->setEnabled(false);
	connect(closeRecordingAction, SIGNAL(triggered()), 
			this, SLOT(closeRecordingWithCheck()));
	fileMenu->addAction(closeRecordingAction);

	/* Windows menu */
	windowsMenu = new QMenu(tr("&Windows"));
	showPlotWindow = new QAction(tr("&Channel view"), windowsMenu);
	showPlotWindow->setShortcut(QKeySequence("Ctrl+0"));
	showPlotWindow->setCheckable(true);
	showPlotWindow->setChecked(true);
	//connect(showPlotWindow, SIGNAL(triggered()), plotWindow, SLOT(toggleVisible()));
	windowsMenu->addAction(showPlotWindow);

	showControlsWindow = new QAction(tr("Control window"), windowsMenu);
	showControlsWindow->setShortcut(QKeySequence("Ctrl+1"));
	showControlsWindow->setCheckable(true);
	showControlsWindow->setChecked(true);
	//connect(showControlsWindow, SIGNAL(triggered()), this, SLOT(toggleVisible()));
	windowsMenu->addAction(showControlsWindow);

	/* eventually same for online analysis and channel inspector */

	/* Add menus to bar and bar to PlotWindow */
	menubar->addMenu(fileMenu);
	menubar->addMenu(windowsMenu);
	setMenuBar(menubar);
}

void MealogWindow::initPlotWindow(void) {
	plotWindow = new PlotWindow(this);
	plotWindow->show();
}

void MealogWindow::createNewRecording(void) {
	statusBar->showMessage("Initializing recording");
	/* Construct the filename from the save directory and the
	 * filename, and check if it exists, asking the user to confirm
	 * an overwrite if it does.
	 */
	QFile path(getFullFilename());
	if (path.exists()) {
		if (!(path.fileName().contains(Mealog::DEFAULT_SAVE_FILE.fileName()))) {
			if (!confirmFileOverwrite(path))
				return;
		}	
		if (!deleteOldRecording(path))
			return;
	}

	/* Construct a recording object and set parameters */
	recording = new H5Recording(path.fileName().toStdString());
	setRecordingParameters();
	recording->flush();
	if (daqClient != nullptr)
		sendDaqsrvInitMessage();

	/* Disable parameter selections */
	setParameterSelectionsEnabled(false);

	/* Enable the "Close" menu option */
	closeRecordingAction->setEnabled(true);

	/* Enable the "Start" button, if connection to the Daqsrv */
	if ((daqClient != nullptr) && (daqClient->isConnected()))
		playButton->setEnabled(true);

	/* Finalize initialisation */
	recordingStatus = Mealog::INITIALIZED | Mealog::NOT_STARTED;
	statusBar->showMessage("Ready");

}

QString MealogWindow::getFullFilename(void) {
	//return QDir::cleanPath(settings.getSaveDir() + "/" + fileLine->text());
	return QDir::cleanPath(Mealog::DEFAULT_SAVE_DIR.absolutePath() 
			+ "/" + fileLine->text());
}

bool MealogWindow::deleteOldRecording(QFile &path) {
	if (!path.remove()) {
		QMessageBox box(this);
		box.setText("Error");
		box.setInformativeText(
				"Could not overwrite the requested file. \
				Remove manually and try again.");
		box.setStandardButtons(QMessageBox::Ok);
		box.setDefaultButton(QMessageBox::Ok);
		box.exec();
		return false;
	}
	return true;
}

void MealogWindow::setRecordingParameters(void) {

	/* Set length and sample information */
	recording->setLength(totalTimeLine->text().toDouble()); // Samples set internally
	recording->setLive(true);
	recording->setLastValidSample(0);

	/* Use defaults */
	recording->setFileType(recording->type());
	recording->setFileVersion(recording->version());
	recording->setSampleRate(recording->sampleRate());

	double adcRange = Mealog::ADC_RANGES.at(adcRangeBox->currentIndex());
	recording->setOffset(adcRange);
	recording->setGain((adcRange * 2) / (1 << 16));

	/* Date and time */
	QString tm(QTime::currentTime().toString("h:mm:ss AP"));
	QString dt(QDate::currentDate().toString("ddd, MMM dd, yyyy"));
	recording->setTime(tm.toStdString());
	recording->setDate(dt.toStdString());
}

void MealogWindow::sendDaqsrvInitMessage(void) {
	daqClient->setLength(recording->length());
	daqClient->setAdcRange(recording->offset());
	daqClient->setBlockSize(BLOCK_SIZE);
	daqClient->setTrigger(triggerBox->currentText());
	daqClient->initExperiment();
}

bool MealogWindow::confirmFileOverwrite(const QFile &path) {
	QMessageBox box(this);
	box.setWindowTitle("File exists"); // ignore on OSX
	box.setText("The selected file already exists.");
	box.setInformativeText("The file \"" + 
			path.fileName() + "\" already exists. Overwrite?");
	box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	box.setDefaultButton(QMessageBox::Cancel);
	return box.exec();
}

void MealogWindow::setParameterSelectionsEnabled(bool enabled) {
	adcRangeBox->setEnabled(enabled);
	triggerBox->setEnabled(enabled);
	totalTimeLine->setEnabled(enabled);
	fileLine->setEnabled(enabled);
	choosePathButton->setEnabled(enabled);
}

void MealogWindow::closeRecordingWithCheck(void) {
	if (recordingStatus & Mealog::STARTED) {
		if (confirmCloseRecording() == QMessageBox::Cancel)
			return;
	}

	closeRecording();
}

void MealogWindow::closeRecording(void) {
	recording->flush();
	delete recording;
	recording = nullptr;
	recordingStatus = Mealog::UNINITIALIZED & Mealog::NOT_STARTED;
	setParameterSelectionsEnabled(true);
	closeRecordingAction->setEnabled(false);
	if (daqClient->isConnected()) {
		daqClient->disconnectFromDaqsrv();
		setNidaqInterfaceEnabled(true);
	}
}

int MealogWindow::confirmCloseRecording(void) {
	QMessageBox box(this);
	box.setWindowTitle("Confirm close");
	box.setText("The recording is currently active.");
	box.setInformativeText("Are you sure you want to close it?");
	box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	box.setDefaultButton(QMessageBox::Cancel);
	return box.exec();
}

void MealogWindow::loadRecording() {

	/* Confirm closing of a current recording */
	if ((recordingStatus & Mealog::STARTED) || 
			(recordingStatus & Mealog::INITIALIZED)) {
		if (confirmCloseRecording() == QMessageBox::Cancel)
			return;
		closeRecording();
	}

	/* Get a new recording file */
	statusBar->showMessage("Loading recording");
	QString filename = QFileDialog::getOpenFileName(
			this, tr("Load recording"),
			STARTING_SAVE_DIR, tr("Recordings (*.h5)"));
	if (filename.isNull()) {
		statusBar->showMessage("Ready");
		return;
	}
	
	/* Open the recording */
	if (recording != nullptr)
		delete recording;
	try {
		recording = new H5Recording(filename.toStdString());
	} catch (std::exception &e) {
		qDebug() << "error creating h5recording";
	}
	plotWindow->recording = recording;
	fileLine->setText(filename.section("/", -1));
	QString path = filename.section("/", 1, -2);
	settings.setSaveFilename(fileLine->text());
	settings.setSaveDir(path);
	setNidaqInterfaceEnabled(false);
	statusBar->showMessage("Ready");
}

void MealogWindow::setNidaqInterfaceEnabled(bool enabled) {
	connectToNidaqButton->setEnabled(enabled);
	nidaqHost->setEnabled(enabled);
	if (!enabled)
		nidaqStatus->setText("Disabled");
}

//void MealogWindow::initServer(void) {
	//server = new QTcpServer(this);
	//server->listen(QHostAddress(Mealog::IPC_HOST), Mealog::IPC_PORT);
	//qDebug() << "Server started at: " << server->serverAddress().toString();
//}

//void MealogWindow::acceptClients(void) {
	//QTcpSocket *socket = server->nextPendingConnection();
	//socket->setParent(this);
	//qDebug() << "New client: " << socket->peerAddress() << ":" << socket->peerPort();
	//connect(socket, SIGNAL(readyRead()), 
			//this, SLOT(respondToClient()));
//}

//bool MealogWindow::readMessage(QTcpSocket *socket, 
		//mearec::RecordingStatusRequest &request) {
	//[> Read the size <]
	//QDataStream stream(socket);
	//quint32 size = 0;
	//stream >> size;
	//if (size == 0)
		//return false;

	//[> Return the parsed message <]
	//return request.ParseFromString(socket->read(size).toStdString());
//}

//bool MealogWindow::writeMessage(QTcpSocket *socket, 
		//mearec::RecordingStatusReply &reply) {
	//[> Write the size <]
	//QDataStream stream(socket);
	//quint32 size = reply.ByteSize();
	//qDebug() << "Writing size (" << size << ") to socket's datastream";
	//stream << size;

	//[> Write the message <]
	//std::string replyData;
	//if (!reply.SerializeToString(&replyData))
		//return false;
	//if (socket->write(replyData.data(), size) != size)
		//return false;
	//socket->flush();
	//return true;
//}

//void MealogWindow::respondToClient(void) {
	//[> Ignore all messages until initialized? <]
	//if (!recordingInitialized) {
		//qDebug() << "Ignoring message from peer, recording is not initialized";
		//return;
	//}
	//QTcpSocket *socket = dynamic_cast<QTcpSocket *>(QObject::sender());
	//mearec::RecordingStatusRequest request;
	//bool ok = readMessage(socket, request);
	//if (!ok) // ignore bad messages for now
		//return;

	//qDebug() << "Received request from peer: " << request.ByteSize() << " bytes";

	//[> Send back the correct reply <]
	//mearec::RecordingStatusReply reply = constructStatusReply(request);
	//qDebug() << "Sending reply: " << reply.ByteSize() << " bytes";
	//writeMessage(socket, reply);
//}

//mearec::RecordingStatusReply MealogWindow::constructStatusReply(
		//mearec::RecordingStatusRequest request) {
	//mearec::RecordingStatusReply reply;
	//if (request.has_status() && request.status())
		//reply.set_status(recordingStatus);
	//if (request.has_filename() && request.filename())
		//reply.set_filename(recording->filename());
	//if (request.has_length() && request.length())
		//reply.set_length(recording->length());
	//if (request.has_nsamples() && request.nsamples())
		//reply.set_nsamples(recording->nsamples());
	//if (request.has_lastvalidsample() && request.lastvalidsample())
		//reply.set_lastvalidsample(recording->lastValidSample());
	//if (request.has_blocksize() && request.blocksize())
		//reply.set_blocksize(recording->blockSize());
	//if (request.has_samplerate() && request.samplerate())
		//reply.set_samplerate(recording->sampleRate());
	//if (request.has_gain() && request.gain())
		//reply.set_gain(recording->gain());
	//if (request.has_offset() && request.offset())
		//reply.set_offset(recording->offset());
	//if (request.has_date() && request.date())
		//reply.set_date(recording->date());
	//return reply;
//}



//void MealogWindow::cleanupRecording(void) {
	//connectButton->setText("Connect");
	//nidaqStatus->setText("Connection to NI-DAQ server ended");
	//isRecording = false;
	//statusBar->showMessage("Recording finished", 10000);
	//startButton->setEnabled(false);
	//delete this->recording;
//}



//void MealogWindow::deInitRecording(void) {
	//statusBar->showMessage("Reseting recording");
	//[> Close the recording file <]
	//delete recording;
	//recording = nullptr;

	//[> Remove the file <]
	//if (!removeOldRecording(*getFullFilename()))
		//return;

	//[> Re-enabled parameter selection <]
	//setParameterSelectionsEnabled(true);

	//[> Change buttons <]
	//startButton->setEnabled(false);
	//initRecordingButton->setText("Initialize");
	//initRecordingButton->setToolTip("Initialize a recording with the given parameters");
	//disconnect(initRecordingButton, SIGNAL(clicked()), 
			//this, SLOT(deInitRecording()));
	//connect(initRecordingButton, SIGNAL(clicked()),
			//this, SLOT(initRecording()));
	//recordingInitialized = false;
	//statusBar->showMessage("Ready");
//}

void MealogWindow::initSignals(void) {
	//connect(this->server, SIGNAL(newConnection()), 
			//this, SLOT(acceptClients()));
	connect(this->choosePathButton, SIGNAL(clicked()),
			this, SLOT(chooseSaveDir()));
	connect(this->connectToNidaqButton, SIGNAL(clicked()),
			this, SLOT(connectToDaqsrv()));
	connect(this->playButton, SIGNAL(clicked()), 
			this, SLOT(startRecording()));
}

void MealogWindow::chooseSaveDir(void) {
	QString dir = QFileDialog::getExistingDirectory(this,
			"Choose save directory",
			settings.getSaveDir(),
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	settings.setSaveDir(dir);
}

//bool MealogWindow::checkMeaviewRunning(void) {
	//QProcess pgrep;
	//pgrep.setProgram("pgrep");
	//QStringList args = {"meaview"};
	//pgrep.setArguments(args);
	//pgrep.start();
	//pgrep.waitForFinished();
	//return (pgrep.readAllStandardOutput().length() > 0);
//}

//void MealogWindow::startMeaview(void) {
	//statusBar->showMessage("Starting meaview");
	//if (checkMeaviewRunning())
		//return;
	//QProcess m;
	//m.startDetached("/Users/bnaecker/FileCabinet/stanford/baccuslab/mearec/meaview/meaview.app/Contents/MacOS/meaview");
	//m.waitForStarted();
	//statusBar->showMessage("Ready");
//}

void MealogWindow::connectToDaqsrv(void) {
	statusBar->showMessage("Connecting to NI-DAQ server");
	nidaqStatus->setText("Connecting ...");
	daqClient = new DaqClient(nidaqHost->text());
	connect(daqClient, SIGNAL(connectionMade(bool)),
			this, SLOT(handleDaqsrvConnection(bool)));
	connect(daqClient, SIGNAL(disconnected()), 
			this, SLOT(handleServerDisconnection()));
	connect(daqClient, SIGNAL(error()), 
			this, SLOT(handleServerError()));
	daqClient->connectToDaqsrv();
	setNidaqInterfaceEnabled(false);
}

void MealogWindow::handleServerDisconnection(void) {
	closeRecording();
	setParameterSelectionsEnabled(true);
	setPlaybackButtonsEnabled(false);
	disconnectFromDaqsrv();
}

void MealogWindow::handleServerError(void) {
	connectToNidaqButton->setText("Connect");
	nidaqStatus->setText("Connection to server interrupted");
	if (recordingStatus & Mealog::STARTED) {
		closeRecording();
		QErrorMessage *errMsg = new QErrorMessage(this);
		errMsg->showMessage("Connection to NI-DAQ server interrupted"\
				", recording terminated!");
	}
	setNidaqInterfaceEnabled(true);
	setPlaybackButtonsEnabled(false);
	setParameterSelectionsEnabled(true);
}

void MealogWindow::disconnectFromDaqsrv(void) {
	daqClient->disconnectFromDaqsrv();
	connectToNidaqButton->setText("Connect");
	nidaqStatus->setText("Not connected");
	setNidaqInterfaceEnabled(true);
	disconnect(connectToNidaqButton, SIGNAL(clicked()), 
			this, SLOT(disconnectFromDaqsrv()));
	connect(connectToNidaqButton, SIGNAL(clicked()),
			this, SLOT(connectToDaqsrv()));
	disconnect(daqClient, SIGNAL(disconnected()), 
			this, SLOT(handleServerDisconnection()));
	disconnect(daqClient, SIGNAL(error()), 
			this, SLOT(handleServerError()));
}

void MealogWindow::handleDaqsrvConnection(bool made) {
	if (made) {
		nidaqStatus->setText("Connected to NI-DAQ");
		connectToNidaqButton->setText("Disconnect");
		connectToNidaqButton->setEnabled(true);
		nidaqHost->setEnabled(false);
		disconnect(connectToNidaqButton, SIGNAL(clicked()), 
				this, SLOT(connectToDaqsrv()));
		connect(connectToNidaqButton, SIGNAL(clicked()),
				this, SLOT(disconnectFromDaqsrv()));
		if (recordingStatus & Mealog::INITIALIZED) {
			sendDaqsrvInitMessage();
			playButton->setEnabled(true);
		}
		statusBar->showMessage("Ready");
	} else {
		nidaqStatus->setText("Error connecting to NI-DAQ, correct IP?");
		delete daqClient;
		daqClient = nullptr;
		setNidaqInterfaceEnabled(true);
		statusBar->showMessage("Ready");
	}
}

void MealogWindow::startRecording(void) {
	connect(daqClient, SIGNAL(dataAvailable()), this, SLOT(recvData()));
	daqClient->startRecording();
	statusBar->showMessage(QString("Recording data to %1").arg(
				getFullFilename()));
}

void MealogWindow::recvData(void) {
	H5Rec::Samples samples(daqClient->nchannels(), daqClient->blockSize());
	daqClient->recvData(samples.memptr());
	recording->setData(numSamplesAcquired, 
			numSamplesAcquired + daqClient->blockSize(), samples);
	numSamplesAcquired += daqClient->blockSize();
	recording->setLastValidSample(numSamplesAcquired);
	emit newDataAvailable();
	qDebug() << numSamplesAcquired << "acquired";
}

