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
#include <QFileInfo>
#include <QPair>

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

void MealogWindow::initSettings(void) {
	settings.setSaveDir(Mealog::DEFAULT_SAVE_DIR.absolutePath());
	settings.setDisplayScale(DEFAULT_DISPLAY_SCALE);
}

void MealogWindow::initGui(void) {
	setWindowTitle("Mealog controls");
	setGeometry(Mealog::WINDOW_XPOS, Mealog::WINDOW_YPOS, 
			Mealog::WINDOW_WIDTH, Mealog::WINDOW_HEIGHT);
	mainLayout = new QGridLayout();

	/* Initialize top portion, with new/load buttons and path info */
	fileLabel = new QLabel("File:", this);
	fileLine = new QLineEdit(Mealog::DEFAULT_SAVE_FILE.fileName(), this);
	fileLine->setToolTip("Name of file to which data is saved");
	fileValidator = new QRegExpValidator(QRegExp("(\\w+[-._]*)+"), this);
	fileLine->setValidator(fileValidator);
	choosePathButton = new QPushButton("Path", this);
	choosePathButton->setToolTip("Choose save directory");

	/* Initialize playback control group */
	playbackGroup = new QGroupBox("Controls", this);
	playbackLayout = new QGridLayout(playbackGroup);
	newRecordingButton = new QPushButton("New", playbackGroup);
	newRecordingButton->setToolTip("Create new recording with given parameters");
	loadRecordingButton = new QPushButton("Load", playbackGroup);
	loadRecordingButton->setToolTip("Load a previous recording for playback");
	closeRecordingButton = new QPushButton("Close", playbackGroup);
	closeRecordingButton->setToolTip("Close the current recording.");
	closeRecordingButton->setEnabled(false);
	timeLabel = new QLabel("Time:", playbackGroup);
	timeLine = new QLineEdit(playbackGroup);
	timeLine->setReadOnly(true);
	totalTimeLine = new QLineEdit(
			QString::number(Mealog::DEFAULT_EXPERIMENT_LENGTH), playbackGroup);
	totalTimeLine->setToolTip("Length of the experiment (seconds)");
	totalTimeValidator = new QIntValidator(1, 
			Mealog::MAX_EXPERIMENT_LENGTH, playbackGroup);
	totalTimeLine->setValidator(totalTimeValidator);
	startButton = new QPushButton("Start", playbackGroup);
	startButton->setToolTip("Play or pause recording. Does not affect saving of data");
	startButton->setShortcut(Qt::Key_Space);
	stopButton = new QPushButton("Stop", playbackGroup);
	stopButton->setToolTip("Stop recording, including the saving of data");
	jumpBackButton = new QPushButton("Back", playbackGroup);
	jumpBackButton->setToolTip("Skip backwards, without affecting saving");
	jumpForwardButton = new QPushButton("Forward", playbackGroup);
	jumpForwardButton->setToolTip("Skip forwards, without affecting saving");
	jumpToBeginningButton = new QPushButton("Start", playbackGroup);
	jumpToBeginningButton->setToolTip("Jump back to the start of the recording."\
			" Does not affect saving.");
	jumpToEndButton = new QPushButton("End", playbackGroup);
	jumpToEndButton->setToolTip("Jump to end of recording or most recent data");
	setPlaybackButtonsEnabled(false);
	setPlaybackMovementButtonsEnabled(false);
	playbackLayout->addWidget(newRecordingButton, 0, 0);
	playbackLayout->addWidget(loadRecordingButton, 0, 1);
	playbackLayout->addWidget(closeRecordingButton, 0, 2);
	playbackLayout->addWidget(fileLabel, 1, 0);
	playbackLayout->addWidget(fileLine, 1, 1);
	playbackLayout->addWidget(choosePathButton, 1, 2);
	playbackLayout->addWidget(timeLabel, 2, 0);
	playbackLayout->addWidget(timeLine, 2, 1);
	playbackLayout->addWidget(totalTimeLine, 2, 2);
	playbackLayout->addWidget(jumpBackButton, 3, 0);
	playbackLayout->addWidget(startButton, 3, 1);
	playbackLayout->addWidget(jumpForwardButton, 3, 2);
	playbackLayout->addWidget(jumpToBeginningButton, 4, 0);
	playbackLayout->addWidget(stopButton, 4, 1);
	playbackLayout->addWidget(jumpToEndButton, 4, 2);
	playbackGroup->setLayout(playbackLayout);
	mainLayout->addWidget(playbackGroup, 0, 0, 1, 2);

	/* Initialize NI-DAQ group */
	nidaqGroup = new QGroupBox("NI-DAQ", this);
	nidaqLayout = new QGridLayout(nidaqGroup);
	connectToNidaqButton = new QPushButton("Connect", nidaqGroup);
	connectToNidaqButton->setToolTip("Connect to NI-DAQ server to initialize the recording");
	nidaqHostLabel = new QLabel("Host:", nidaqGroup);
	nidaqHostLabel->setAlignment(Qt::AlignRight);
	nidaqHost = new QLineEdit(Mealog::DEFAULT_NIDAQ_HOST, nidaqGroup);
	nidaqHost->setToolTip("IP address of the computer running the NI-DAQ server");
	nidaqValidator = new QRegExpValidator(
			QRegExp("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}"\
				"([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])"), nidaqGroup);
	nidaqHost->setValidator(nidaqValidator);
	nidaqStatusLabel = new QLabel("Status:", nidaqGroup);
	nidaqStatusLabel->setAlignment(Qt::AlignRight);
	nidaqStatus = new QLineEdit("Not connected", nidaqGroup);
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
	recordingGroup = new QGroupBox("Recording parameters", this);
	recordingLayout = new QGridLayout(recordingGroup);
	adcRangeLabel = new QLabel("ADC range:", recordingGroup);
	adcRangeBox = new QComboBox(recordingGroup);
	for (auto &each : Mealog::ADC_RANGES)
		adcRangeBox->addItem(QString::number(each), QVariant(each));
	adcRangeBox->setToolTip("Set the voltage range of the NI-DAQ card");
	adcRangeBox->setCurrentIndex(
			Mealog::ADC_RANGES.indexOf(Mealog::DEFAULT_ADC_RANGE));
	triggerBox = new QComboBox(recordingGroup);
	triggerBox->addItems(Mealog::TRIGGERS);
	triggerLabel = new QLabel("Trigger:", recordingGroup);
	triggerBox->setToolTip("Set the triggering mechanism for starting the experiment");
	recordingLayout->addWidget(adcRangeLabel, 0, 0);
	recordingLayout->addWidget(adcRangeBox, 0, 1);
	recordingLayout->addWidget(triggerLabel, 1, 0);
	recordingLayout->addWidget(triggerBox, 1, 1);
	recordingGroup->setLayout(recordingLayout);
	mainLayout->addWidget(recordingGroup, 1, 2);

	/* Initialize group with parameters of the data display window */
	displayGroup = new QGroupBox("Display", this);
	displayLayout = new QGridLayout(displayGroup);
	refreshLabel = new QLabel("Refresh", displayGroup);
	refreshBox = new QSpinBox(displayGroup);
	refreshBox->setSingleStep(100);
	refreshBox->setRange(MIN_REFRESH_INTERVAL, MAX_REFRESH_INTERVAL);
	refreshBox->setSuffix(" ms");
	refreshBox->setValue(settings.getRefreshInterval());
	refreshBox->setToolTip("Interval at which plots refresh");
	viewLabel = new QLabel("View:", displayGroup);
	viewBox = new QComboBox(displayGroup);
	viewBox->setToolTip("Set arrangement of subplots to match array");
	for (auto &view : CHANNEL_VIEW_STRINGS)
		viewBox->addItem(view);
	viewBox->setCurrentIndex(
			viewBox->findText(settings.getChannelViewString()));
	jumpSizeLabel = new QLabel("Jump size:", displayGroup);
	jumpSizeBox = new QSpinBox(displayGroup);
	jumpSizeBox->setToolTip("Set size of a jump when skipping forward"\
			" or backward through recording");
	jumpSizeBox->setRange(JUMP_MIN, JUMP_MAX);
	jumpSizeBox->setSingleStep(JUMP_STEP_SIZE);
	jumpSizeBox->setValue(settings.getRefreshInterval());
	jumpSizeBox->setSuffix(" ms");
	scaleLabel = new QLabel("Scale:", displayGroup);
	scaleBox = new QComboBox(displayGroup);
	scaleBox->setToolTip("Change y-axis scaling, larger means larger axis extent");
	for (auto &scale : DISPLAY_SCALES)
		scaleBox->addItem(QString::number(scale));
	scaleBox->setCurrentIndex(
			settings.getDisplayScales().indexOf(DEFAULT_DISPLAY_SCALE));
	autoscaleLabel = new QLabel("Autoscale:", displayGroup);
	autoscaleBox = new QCheckBox(displayGroup);
	autoscaleBox->setToolTip("If checked, each subplot will scale to fit its data");
	autoscaleBox->setTristate(false);
	autoscaleBox->setChecked(false);
	automeanLabel = new QLabel("Automean:", displayGroup);
	automeanBox = new QCheckBox(displayGroup);
	automeanBox->setToolTip("If checked, mean-subtract each plot individually");
	automeanBox->setTristate(false);
	automeanBox->setChecked(false);
	displayLayout->addWidget(refreshLabel, 0, 0);
	displayLayout->addWidget(refreshBox, 0, 1);
	displayLayout->addWidget(viewLabel, 1, 0);
	displayLayout->addWidget(viewBox, 1, 1);
	displayLayout->addWidget(jumpSizeLabel, 2, 0);
	displayLayout->addWidget(jumpSizeBox, 2, 1);
	displayLayout->addWidget(scaleLabel, 3, 0);
	displayLayout->addWidget(scaleBox, 3, 1);
	displayLayout->addWidget(autoscaleLabel, 4, 0);
	displayLayout->addWidget(autoscaleBox, 4, 1);
	displayLayout->addWidget(automeanLabel, 5, 0);
	displayLayout->addWidget(automeanBox, 5, 1);
	displayGroup->setLayout(displayLayout);
	mainLayout->addWidget(displayGroup, 0, 2);

	statusBar = new QStatusBar(this);
	statusBar->showMessage("Ready");
	this->setStatusBar(statusBar);
	this->setCentralWidget(new QWidget(this));
	this->centralWidget()->setLayout(mainLayout);
}

void MealogWindow::setPlaybackButtonsEnabled(bool enabled) {
	startButton->setEnabled(enabled);
	stopButton->setEnabled(enabled);
}

void MealogWindow::setPlaybackMovementButtonsEnabled(bool enabled) {
	jumpBackButton->setEnabled(enabled);
	jumpForwardButton->setEnabled(enabled);
	jumpToBeginningButton->setEnabled(enabled);
	jumpToEndButton->setEnabled(enabled);
}

void MealogWindow::initMenuBar(void) {
	menubar = new QMenuBar(0);

	/* About mealog */
	aboutAction = new QAction(tr("about.*"), menubar);
	aboutAction->setMenuRole(QAction::AboutRole);

	/* File menu */
	fileMenu = new QMenu(tr("&File"));

	/* New recording menu item */
	newRecordingAction = new QAction(tr("&New"), fileMenu);
	newRecordingAction->setShortcut(QKeySequence("Ctrl+N"));
	fileMenu->addAction(newRecordingAction);

	/* Load recording for replay */
	loadRecordingAction = new QAction(tr("&Open"), fileMenu);
	loadRecordingAction->setShortcut(QKeySequence("Ctrl+O"));
	fileMenu->addAction(loadRecordingAction);

	/* Close recording */
	closeRecordingAction = new QAction(tr("&Close"), fileMenu);
	closeRecordingAction->setShortcut(QKeySequence("Ctrl+C"));
	closeRecordingAction->setEnabled(false);
	fileMenu->addAction(closeRecordingAction);

	/* Windows menu */
	windowsMenu = new QMenu(tr("&Windows"));
	showPlotWindow = new QAction(tr("&Channel view"), windowsMenu);
	showPlotWindow->setShortcut(QKeySequence("Ctrl+0"));
	showPlotWindow->setCheckable(true);
	showPlotWindow->setChecked(true);
	windowsMenu->addAction(showPlotWindow);

	showControlsWindow = new QAction(tr("Control window"), windowsMenu);
	showControlsWindow->setShortcut(QKeySequence("Ctrl+1"));
	showControlsWindow->setCheckable(true);
	showControlsWindow->setChecked(true);
	windowsMenu->addAction(showControlsWindow);

	/* eventually same for online analysis and channel inspector */

	/* Add menus to bar and bar to PlotWindow */
	menubar->addMenu(fileMenu);
	menubar->addMenu(windowsMenu);
	setMenuBar(menubar);
}

void MealogWindow::initPlotWindow(void) {
	QPair<int, int> p = CHANNEL_COL_ROW_MAP.value(settings.getChannelViewString());
	plotWindow = new PlotWindow(p.first, p.second, this);
	plotWindow->show();
	lastSamplePlotted = 0;
}

void MealogWindow::createNewRecording(void) {

	/* Confirm closing of a current recording */
	if ((recordingStatus & Mealog::STARTED) || 
			(recordingStatus & Mealog::INITIALIZED)) {
		if (confirmCloseRecording() == QMessageBox::Cancel)
			return;
		closeRecording();
	}

	statusBar->showMessage("Initializing recording");
	/* Construct the filename from the save directory and the
	 * filename, and check if it exists, asking the user to confirm
	 * an overwrite if it does.
	 */
	QFile path(getFullFilename());
	if (path.exists()) {
		if (!isDefaultSaveFile()) {
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
	closeRecordingButton->setEnabled(true);

	/* Enable the "Start" button, if connection to the Daqsrv */
	if ((daqClient != nullptr) && (daqClient->isConnected()))
		startButton->setEnabled(true);

	/* Finalize initialisation */
	recordingStatus = (
			Mealog::INITIALIZED | 
			Mealog::NOT_STARTED | 
			Mealog::RECORDING
		);
	newRecordingButton->setEnabled(false);
	newRecordingAction->setEnabled(false);
	statusBar->showMessage("Ready");
	startButton->setShortcut(Qt::Key_Space);
}

QString MealogWindow::getFullFilename(void) {
	QString name(fileLine->text());
	if (!name.endsWith(".h5")) {
		name.append(".h5");
		fileLine->setText(name);
	}
	return settings.getSaveDir() + "/" + name;
}

bool MealogWindow::isDefaultSaveFile(void) {
	return ( (fileLine->text() == Mealog::DEFAULT_SAVE_FILE.fileName()) && 
			(settings.getSaveDir() == Mealog::DEFAULT_SAVE_DIR.dirName()));
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
	//recording->setFileType(recording->type());
	//recording->setFileVersion(recording->version());
	//recording->setSampleRate(recording->sampleRate());

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
	totalTimeLine->setReadOnly(!enabled);
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
	if (recordingStatus & Mealog::INITIALIZED) {
		recording->setLive(false);
		recording->flush();
		delete recording;
		recording = nullptr;
		if (recordingStatus & Mealog::PLAYBACK) {
			playbackTimer->stop();
			delete playbackTimer;
			playbackTimer = nullptr;
		}
		if (recordingStatus & Mealog::NOT_STARTED)
			QFile::remove(getFullFilename());
	}
	statusBar->showMessage("Ready");
	plotWindow->clearAll();
	recordingStatus = Mealog::UNINITIALIZED | Mealog::NOT_STARTED;
	lastSamplePlotted = 0;
	numSamplesAcquired = 0;

	/* Reset recording parameters */
	timeLine->setText("");
	totalTimeLine->setText(
			QString::number(Mealog::DEFAULT_EXPERIMENT_LENGTH));
	fileLine->setText(
			Mealog::DEFAULT_SAVE_FILE.fileName().section("/", -1));
	triggerBox->setCurrentIndex(0);
	adcRangeBox->setCurrentIndex(
			Mealog::ADC_RANGES.indexOf(Mealog::DEFAULT_ADC_RANGE));
	setParameterSelectionsEnabled(true);

	/* Re-enable GUI components */
	startButton->setText("Start");
	disconnect(startButton, &QPushButton::clicked,
			this, &MealogWindow::pauseRecording);
	connect(startButton, &QPushButton::clicked,
			this, &MealogWindow::startRecording);
	startButton->setShortcut(Qt::Key_Space);
	setPlaybackButtonsEnabled(false);
	setPlaybackMovementButtonsEnabled(false);
	closeRecordingAction->setEnabled(false);
	closeRecordingButton->setEnabled(false);
	setNidaqInterfaceEnabled(true);
	newRecordingButton->setEnabled(true);
	newRecordingAction->setEnabled(true);

	/* Disconnect NI-DAQ client, if this is a recording */
	if ((daqClient != nullptr) && (daqClient->isConnected())) {
		disconnect(daqClient, &DaqClient::disconnected,
				this, &MealogWindow::handleServerDisconnection);
		disconnect(daqClient, &DaqClient::error,
				this, &MealogWindow::handleServerError);
		disconnect(connectToNidaqButton, &QPushButton::clicked,
				this, &MealogWindow::disconnectFromDaqsrv);
		daqClient->disconnectFromDaqsrv();
		delete daqClient;
		daqClient = nullptr;
		connect(connectToNidaqButton, &QPushButton::clicked,
				this, &MealogWindow::connectToDaqsrv);
		connect(connectToNidaqButton, &QPushButton::clicked,
				this, &MealogWindow::connectToDaqsrv);
	}
	nidaqStatus->setText("Not connected");
	connectToNidaqButton->setText("Connect");
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
			settings.getSaveDir(), tr("Recordings (*.h5)"));
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
	fileLine->setText(filename.section("/", -1));
	QString path = filename.section("/", 1, -2);
	settings.setSaveFilename(fileLine->text());
	recordingStatus = (
			Mealog::INITIALIZED |
			Mealog::NOT_STARTED |
			Mealog::PLAYBACK
		);
	initPlayback();
	settings.setSaveDir(path);
	setNidaqInterfaceEnabled(false);
	setParameterSelectionsEnabled(false);
	closeRecordingAction->setEnabled(true);
	closeRecordingButton->setEnabled(true);

	/* Set parameters of the recording from the file */
	totalTimeLine->setText(QString::number(recording->length()));
	totalTimeLine->setReadOnly(true);
	adcRangeBox->setCurrentIndex(
			Mealog::ADC_RANGES.indexOf(-(recording->offset())));
	triggerBox->setCurrentIndex(Mealog::TRIGGERS.length() - 1);
	statusBar->showMessage("Ready");
}

void MealogWindow::initPlayback() {
	playbackTimer = new QTimer(this);
	playbackTimer->setInterval(settings.getRefreshInterval());
	connect(playbackTimer, &QTimer::timeout,
			this, &MealogWindow::plotNextPlaybackDataBlock);
	startButton->setEnabled(true);
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
	connect(choosePathButton, &QPushButton::clicked,
			this, &MealogWindow::chooseSaveDir);
	connect(connectToNidaqButton, &QPushButton::clicked,
			this, &MealogWindow::connectToDaqsrv);
	connect(startButton, &QPushButton::clicked,
			this, &MealogWindow::startRecording);
	connect(showPlotWindow, &QAction::triggered,
			plotWindow, &PlotWindow::toggleVisible);
	connect(showControlsWindow, &QAction::toggled,
			this, &MealogWindow::setVisible);
	connect(this, &MealogWindow::newDataAvailable,
			this, &MealogWindow::checkReadyForPlotting);
	connect(newRecordingAction, &QAction::triggered,
			this, &MealogWindow::createNewRecording);
	connect(loadRecordingAction, &QAction::triggered,
			this, &MealogWindow::loadRecording);
	connect(closeRecordingAction, &QAction::triggered,
			this, &MealogWindow::closeRecordingWithCheck);
	connect(newRecordingButton, &QPushButton::clicked,
			this, &MealogWindow::createNewRecording);
	connect(loadRecordingButton, &QPushButton::clicked,
			this, &MealogWindow::loadRecording);
	connect(closeRecordingButton, &QPushButton::clicked,
			this, &MealogWindow::closeRecordingWithCheck);
	connect(jumpBackButton, &QPushButton::clicked,
			this, &MealogWindow::jumpBackward);
	connect(jumpForwardButton, &QPushButton::clicked,
			this, &MealogWindow::jumpForward);
	connect(jumpToBeginningButton, &QPushButton::clicked,
			this, &MealogWindow::jumpToBeginning);
	connect(jumpToEndButton, &QPushButton::clicked,
			this, &MealogWindow::jumpToEnd);
	connect(refreshBox, SIGNAL(valueChanged(int)),
			this, SLOT(updateRefreshInterval(int)));
	connect(jumpSizeBox, SIGNAL(valueChanged(int)),
			this, SLOT(updateJumpSize(int)));
	connect(viewBox, &QComboBox::currentTextChanged,
			this, &MealogWindow::updateChannelView);
	connect(viewBox, &QComboBox::currentTextChanged,
			plotWindow, &PlotWindow::updateChannelView);
	connect(autoscaleBox, &QCheckBox::stateChanged,
			this, &MealogWindow::updateAutoscale);
	connect(automeanBox, &QCheckBox::stateChanged,
			this, &MealogWindow::updateAutomean);
	connect(scaleBox, &QComboBox::currentTextChanged,
			this, &MealogWindow::updateDisplayScale);
	connect(this, &MealogWindow::recordingFinished,
			this, &MealogWindow::endRecording);
}

void MealogWindow::chooseSaveDir(void) {
	QString dir = QFileDialog::getExistingDirectory(this,
			"Choose save directory",
			settings.getSaveDir(),
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isNull())
		return;
	QFileInfo info(dir);
	if (!info.isWritable()) {
		QMessageBox::critical(this, 
				"Permissions error",
				"You don't have write permissions to the chosen directory");
		return;
	}
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
	connect(daqClient, &DaqClient::connectionMade,
			this, &MealogWindow::handleDaqsrvConnection);
	disconnect(connectToNidaqButton, &QPushButton::clicked,
			this, &MealogWindow::connectToDaqsrv);
	daqClient->connectToDaqsrv();
	setNidaqInterfaceEnabled(false);
}

void MealogWindow::handleServerDisconnection(void) {
	setParameterSelectionsEnabled(true);
	setPlaybackButtonsEnabled(false);
	setPlaybackMovementButtonsEnabled(false);
	disconnectFromDaqsrv();
	closeRecording();
}

void MealogWindow::handleServerError(void) {
	connectToNidaqButton->setText("Connect");
	nidaqStatus->setText("Connection to server interrupted");
	if (recordingStatus & Mealog::STARTED) {
		closeRecording();
		QMessageBox::critical(this, "Connection interrupted",
				"Connection to NI-DAQ server application interrupted."\
				" Recording has been stopped, with all previous data "\
				"written to disk.");
	}
	setNidaqInterfaceEnabled(true);
	setPlaybackButtonsEnabled(false);
	setPlaybackMovementButtonsEnabled(false);
	setParameterSelectionsEnabled(true);
	startButton->setText("Start");
	statusBar->showMessage("Ready");
	disconnect(connectToNidaqButton, &QPushButton::clicked,
			this, &MealogWindow::disconnectFromDaqsrv);
	connect(connectToNidaqButton, &QPushButton::clicked,
			this, &MealogWindow::connectToDaqsrv);
	disconnect(daqClient, &DaqClient::disconnected,
			this, &MealogWindow::handleServerDisconnection);
	disconnect(daqClient, &DaqClient::error,
			this, &MealogWindow::handleServerError);
	delete daqClient;
}

void MealogWindow::disconnectFromDaqsrv(void) {
	if (recordingStatus & Mealog::STARTED) {
		if (confirmCloseRecording() == QMessageBox::Cancel)
			return;
	}
	connectToNidaqButton->setText("Connect");
	nidaqStatus->setText("Not connected");
	setNidaqInterfaceEnabled(true);
	statusBar->showMessage("Ready");
	disconnect(connectToNidaqButton, &QPushButton::clicked,
			this, &MealogWindow::disconnectFromDaqsrv);
	connect(connectToNidaqButton, &QPushButton::clicked,
			this, &MealogWindow::connectToDaqsrv);
	disconnect(daqClient, &DaqClient::disconnected,
			this, &MealogWindow::handleServerDisconnection);
	disconnect(daqClient, &DaqClient::error,
			this, &MealogWindow::handleServerError);
	daqClient->disconnectFromDaqsrv();
	closeRecording();
}

void MealogWindow::handleDaqsrvConnection(bool made) {
	if (made) {
		nidaqStatus->setText("Connected to NI-DAQ");
		connectToNidaqButton->setText("Disconnect");
		connectToNidaqButton->setEnabled(true);
		nidaqHost->setEnabled(false);
		connect(connectToNidaqButton, &QPushButton::clicked,
				this, &MealogWindow::disconnectFromDaqsrv);
		connect(daqClient, &DaqClient::disconnected,
				this, &MealogWindow::handleServerDisconnection);
		connect(daqClient, &DaqClient::error,
				this, &MealogWindow::handleServerError);
		if (recordingStatus & Mealog::INITIALIZED) {
			sendDaqsrvInitMessage();
			startButton->setEnabled(true);
		}
		statusBar->showMessage("Ready");
	} else {
		disconnect(connectToNidaqButton, &QPushButton::clicked,
				this, &MealogWindow::disconnectFromDaqsrv);
		connect(connectToNidaqButton, &QPushButton::clicked,
				this, &MealogWindow::connectToDaqsrv);
		nidaqStatus->setText("Error connecting to NI-DAQ, correct IP?");
		delete daqClient;
		daqClient = nullptr;
		setNidaqInterfaceEnabled(true);
		statusBar->showMessage("Ready");
	}
}

void MealogWindow::startRecording(void) {
	if (recordingStatus & Mealog::RECORDING) {
		connect(daqClient, &DaqClient::dataAvailable,
				this, &MealogWindow::recvData);
		daqClient->startRecording();
		statusBar->showMessage(QString("Recording data to %1").arg(
					getFullFilename()));
		recordingStatus = (
				Mealog::INITIALIZED | 
				Mealog::STARTED | 
				Mealog::RECORDING |
				Mealog::PLAYING
			);
	} else {
		playbackTimer->start();
		statusBar->showMessage(QString("Playing back data from %1").arg(
					getFullFilename()));
		recordingStatus = (
				Mealog::INITIALIZED | 
				Mealog::STARTED | 
				Mealog::PLAYBACK | 
				Mealog::PLAYING
			);
	}
	setPlaybackButtonsEnabled(true);
	startButton->setText("Pause");
	disconnect(startButton, &QPushButton::clicked,
				this, &MealogWindow::startRecording);
	connect(startButton, &QPushButton::clicked,
			this, &MealogWindow::pauseRecording);
	connect(stopButton, &QPushButton::clicked,
			this, &MealogWindow::closeRecordingWithCheck);
	startButton->setShortcut(Qt::Key_Space);
}

void MealogWindow::pauseRecording(void) {
	setPlaybackMovementButtonsEnabled(true);
	if (recordingStatus & Mealog::PLAYBACK) {
		playbackTimer->stop();
		statusBar->showMessage("Playback paused");
	} else { 
		disconnect(this, &MealogWindow::newDataAvailable,
				this, &MealogWindow::checkReadyForPlotting);
		statusBar->showMessage("Display paused, data still recording");
	}
	startButton->setText("Start");
	disconnect(startButton, &QPushButton::clicked,
			this, &MealogWindow::pauseRecording);
	connect(startButton, &QPushButton::clicked,
			this, &MealogWindow::restartRecording);
	startButton->setShortcut(Qt::Key_Space);
	uint16_t oldStatus = recordingStatus & ~Mealog::PLAYING;
	recordingStatus = oldStatus | Mealog::PAUSED;
}

void MealogWindow::restartRecording(void) {
	setPlaybackMovementButtonsEnabled(false);
	if (recordingStatus & Mealog::PLAYBACK) {
		playbackTimer->start();
		statusBar->showMessage(QString("Playing back data from %1").arg(
					getFullFilename()));
	} else { 
		statusBar->showMessage(QString("Recording data to %1").arg(
					getFullFilename()));
		int blockSize = ((settings.getRefreshInterval() / 1000) * 
				recording->sampleRate());
		lastSamplePlotted = (numSamplesAcquired / blockSize) * blockSize;
		connect(this, &MealogWindow::newDataAvailable,
				this, &MealogWindow::checkReadyForPlotting);
	}
	startButton->setText("Pause");
	disconnect(startButton, &QPushButton::clicked,
			this, &MealogWindow::restartRecording);
	connect(startButton, &QPushButton::clicked,
			this, &MealogWindow::pauseRecording);
	startButton->setShortcut(Qt::Key_Space);
	uint16_t oldStatus = recordingStatus & ~Mealog::PAUSED;
	recordingStatus = oldStatus | Mealog::PLAYING;
}

void MealogWindow::recvData(qint64 nsamples) {
	H5Rec::Samples samples(nsamples, daqClient->nchannels());
	daqClient->recvData(nsamples, samples.memptr());
	recording->setData(numSamplesAcquired, 
			numSamplesAcquired + nsamples, samples);
	numSamplesAcquired += nsamples;
	recording->setLastValidSample(numSamplesAcquired);
	emit newDataAvailable();
	if (numSamplesAcquired == recording->nsamples())
		emit recordingFinished();
}

void MealogWindow::checkReadyForPlotting(void) {
	size_t numSamplesPerPlotBlock = ((settings.getRefreshInterval() / 1000) *
			recording->sampleRate());
	size_t numNewSamples = (numSamplesAcquired - lastSamplePlotted);
	if (numNewSamples >= numSamplesPerPlotBlock) {
		plotDataBlock(lastSamplePlotted, 
				lastSamplePlotted + numSamplesPerPlotBlock);
	}
}

void MealogWindow::plotDataBlock(uint64_t start, uint64_t end) {
	size_t nsamples = end - start;
	H5Rec::Samples s = recording->data(start, end);
	plotWindow->plotData(s);
	lastSamplePlotted += nsamples;
	updateTime();
	if ((recordingStatus & Mealog::PLAYBACK) && 
			(recordingStatus & Mealog::PLAYING)) {
		if (lastSamplePlotted == recording->nsamples())
			emit recordingFinished();
	}
}

void MealogWindow::plotNextPlaybackDataBlock(void) {
	size_t numSamplesPerPlotBlock = ((settings.getRefreshInterval() / 1000) *
			H5Rec::SAMPLE_RATE);
	if (lastSamplePlotted == recording->nsamples()) {
		emit recordingFinished();
		return;
	}
	plotDataBlock(lastSamplePlotted, lastSamplePlotted + numSamplesPerPlotBlock);
}

void MealogWindow::updateTime(void) {
	double sampleRate = recording->sampleRate();
	double offset = (((double) settings.getRefreshInterval()) / 1000) * sampleRate;
	double sample = lastSamplePlotted;
	timeLine->setText(QString("%1 - %2").arg(
				(sample - offset) / sampleRate, 0, 'f', 1).arg(
				sample / recording->sampleRate(), 0, 'f', 1));
}

void MealogWindow::jumpForward(void) {
	size_t numSamplesPerPlotBlock = ((settings.getRefreshInterval() / 1000) *
			H5Rec::SAMPLE_RATE);
	if (recordingStatus & Mealog::PLAYBACK) {
		plotDataBlock(lastSamplePlotted, 
				lastSamplePlotted + numSamplesPerPlotBlock);
	} else {
		if ((numSamplesAcquired - lastSamplePlotted) > numSamplesPerPlotBlock) {
			plotDataBlock(lastSamplePlotted, 
					lastSamplePlotted + numSamplesPerPlotBlock);
		}
	}
}

void MealogWindow::jumpBackward(void) {
	size_t numSamplesPerPlotBlock = ((settings.getRefreshInterval() / 1000) *
			H5Rec::SAMPLE_RATE);
	if (lastSamplePlotted > numSamplesPerPlotBlock) {
		lastSamplePlotted -= 2 * numSamplesPerPlotBlock;
		plotDataBlock(lastSamplePlotted,
				lastSamplePlotted + numSamplesPerPlotBlock);
	}
}

void MealogWindow::jumpToBeginning(void) {
	size_t numSamplesPerPlotBlock = ((settings.getRefreshInterval() / 1000) *
			H5Rec::SAMPLE_RATE);
	lastSamplePlotted = 0;
	plotDataBlock(lastSamplePlotted, lastSamplePlotted + numSamplesPerPlotBlock);
}

void MealogWindow::jumpToEnd(void) {
	size_t numSamplesPerPlotBlock = ((settings.getRefreshInterval() / 1000) *
			H5Rec::SAMPLE_RATE);
	if (recordingStatus & Mealog::RECORDING) {
		lastSamplePlotted = ((numSamplesAcquired / numSamplesPerPlotBlock) * 
				numSamplesPerPlotBlock);
	} else {
		lastSamplePlotted = recording->nsamples() - numSamplesPerPlotBlock;
	}
	plotDataBlock(lastSamplePlotted, lastSamplePlotted + numSamplesPerPlotBlock);
}

void MealogWindow::updateRefreshInterval(int val) {
	settings.setRefreshInterval(val);
	if (recordingStatus & Mealog::PLAYBACK)
		playbackTimer->setInterval(settings.getRefreshInterval());
}

void MealogWindow::updateJumpSize(int val) {
	settings.setJump(val);
}

void MealogWindow::updateChannelView(const QString &text) {
	settings.setChannelView(text);
}

void MealogWindow::updateAutoscale(int state) {
	settings.setAutoscale(state);
	scaleBox->setEnabled(!state);
}

void MealogWindow::updateAutomean(int state) {
	settings.setAutoMean(state);
}

void MealogWindow::updateDisplayScale(const QString &text) {
	settings.setDisplayScale(DISPLAY_SCALES.at(text.toUInt()));
}

void MealogWindow::endRecording(void) {
	//waitForRecordingFinish();
	closeRecording(); // Waits for plotting threads inside
	recordingStatus = Mealog::UNINITIALIZED | Mealog::FINISHED;
}

void MealogWindow::waitForRecordingFinish(void) {
	plotWindow->forceReplot();
	QCoreApplication::processEvents();
	QThread::msleep(Mealog::RECORDING_FINISH_WAIT_TIME);
}

