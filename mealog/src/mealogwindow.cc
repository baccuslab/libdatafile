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
#include <QDateTime>
#include <QDataStream>
#include <QFileDialog>
#include <QFileInfo>
#include <QPair>

#include <armadillo>

#include "mealogwindow.h"

MealogWindow::MealogWindow(const QString& a, QWidget *parent) 
	: QMainWindow(parent) {
	if (a.toLower() == "hidens") {
		array = "hidens";
		dataSource = Mealog::HIDENS;
		defaultHost = mcsclient::DEFAULT_MCS_HOST;
	} else {
		array = "mcs";
		dataSource = Mealog::MCS;
		defaultHost = hdclient::DEFAULT_HIDENS_HOST;
	}
	initSettings();
	initGui();
	initMenuBar();
	initPlotWindow();
	initOAWindow();
	//initServer();
	initSignals();
}

MealogWindow::~MealogWindow() {
}

void MealogWindow::initSettings(void) {
	settings.setSaveDir(Mealog::DEFAULT_SAVE_DIR.absolutePath());
	settings.setSaveFilename(Mealog::DEFAULT_SAVE_FILE.fileName());
	settings.setDisplayScale(DEFAULT_DISPLAY_SCALE);
	settings.setPlotPen(PLOT_PEN);
	settings.setRefreshInterval(DEFAULT_REFRESH_INTERVAL);
	settings.setChannelView(DEFAULT_VIEW);
	settings.setExperimentLength(Mealog::DEFAULT_EXPERIMENT_LENGTH);
	settings.setAutoscale(false);
	settings.setAutoMean(false);
	settings.setOnlineAnalysisLength(DEFAULT_ONLINE_ANALYSIS_LENGTH);
	settings.setJump(DEFAULT_JUMP_SIZE);
	settings.setNumRows(CHANNEL_COL_ROW_MAP.value(DEFAULT_VIEW).first);
	settings.setNumCols(CHANNEL_COL_ROW_MAP.value(DEFAULT_VIEW).second);
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
	serverGroup = new QGroupBox("Data server", this);
	serverLayout = new QGridLayout(serverGroup);
	connectToServerButton = new QPushButton("Connect", serverGroup);
	connectToServerButton->setToolTip("Connect to data server to initialize the recording");
	serverHostLabel = new QLabel("Host:", serverGroup);
	serverHostLabel->setAlignment(Qt::AlignRight);
	serverHost = new QLineEdit(defaultHost, serverGroup);
	serverHost->setToolTip("IP address of the computer running the data server");
	serverValidator = new QRegExpValidator(
			QRegExp("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}"\
				"([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])"), serverGroup);
	serverHost->setValidator(serverValidator);
	serverStatusLabel = new QLabel("Status:", serverGroup);
	serverStatusLabel->setAlignment(Qt::AlignRight);
	serverStatus = new QLineEdit("Not connected", serverGroup);
	serverStatus->setReadOnly(true);
	serverStatus->setToolTip("Status of connection with data server");
	serverLayout->addWidget(serverHostLabel, 0, 0);
	serverLayout->addWidget(serverHost, 0, 1, 1, 2);
	serverLayout->addWidget(connectToServerButton, 0, 3);
	serverLayout->addWidget(serverStatusLabel, 1, 0);
	serverLayout->addWidget(serverStatus, 1, 1, 1, 3);
	serverGroup->setLayout(serverLayout);
	mainLayout->addWidget(serverGroup, 1, 0, 1, 2);

	/* Initialize group with parameters of the recording */
	
	/* XXX: Parameters are different depending on array type.
	 * Keep ADC range for NIDAQ, swap with a configuration 
	 * setting for the HiDens system
	 */
	recordingGroup = new QGroupBox("Recording parameters", this);
	recordingLayout = new QGridLayout(recordingGroup);
	triggerBox = new QComboBox(recordingGroup);
	triggerBox->addItems(Mealog::TRIGGERS);
	triggerLabel = new QLabel("Trigger:", recordingGroup);
	triggerBox->setToolTip("Set the triggering mechanism for starting the experiment");
	if (array == "hidens") {
		configLabel = new QLabel("Configuration:", recordingGroup);
		chooseConfigButton = new QPushButton("Choose", recordingGroup);
		connect(chooseConfigButton, &QPushButton::clicked,
				this, &MealogWindow::chooseHidensConfiguration);
		configLine = new QLineEdit("", recordingGroup);
		configLine->setReadOnly(true);
		recordingLayout->addWidget(configLabel, 0, 0);
		recordingLayout->addWidget(chooseConfigButton, 0, 1);
		recordingLayout->addWidget(configLine, 1, 0, 1, 2);
		recordingLayout->addWidget(triggerLabel, 2, 0);
		recordingLayout->addWidget(triggerBox, 2, 1);
	} else {
		adcRangeLabel = new QLabel("ADC range:", recordingGroup);
		adcRangeBox = new QComboBox(recordingGroup);
		for (auto &each : Mealog::ADC_RANGES)
			adcRangeBox->addItem(QString::number(each), QVariant(each));
		adcRangeBox->setToolTip("Set the voltage range of the NI-DAQ card");
		adcRangeBox->setCurrentIndex(
				Mealog::ADC_RANGES.indexOf(Mealog::DEFAULT_ADC_RANGE));
		recordingLayout->addWidget(adcRangeLabel, 0, 0);
		recordingLayout->addWidget(adcRangeBox, 0, 1);
		recordingLayout->addWidget(triggerLabel, 1, 0);
		recordingLayout->addWidget(triggerBox, 1, 1);
	}
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
	displayLayout->addWidget(jumpSizeLabel, 1, 0);
	displayLayout->addWidget(jumpSizeBox, 1, 1);
	displayLayout->addWidget(viewLabel, 2, 0);
	displayLayout->addWidget(viewBox, 2, 1);
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

	showOAWindow = new QAction(tr("Online analysis"), windowsMenu);
	showOAWindow->setShortcut(QKeySequence("Ctrl+2"));
	showOAWindow->setCheckable(true);
	showOAWindow->setChecked(false);
	windowsMenu->addAction(showOAWindow);

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

void MealogWindow::initOAWindow(void)
{
	oawindow = new OAWindow(this);
	//oawindow->move(Mealog::WINDOW_XPOS, Mealog::WINDOW_HEIGHT - 10);
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
			if (!confirmFileOverwrite(path)) {
				statusBar->showMessage("Ready");
				return;
			}
		}
		if (!deleteOldRecording(path)) {
			statusBar->showMessage("Ready");
			return;
		}
	}

	/* Construct a recording object and set parameters */
	recording = new H5Rec::H5Recording(path.fileName().toStdString());
	setRecordingParameters();
	recording->flush();
	if (dataClient)
		initDataServer();

	/* Disable parameter selections */
	setParameterSelectionsEnabled(false);

	/* Enable the "Close" menu option */
	closeRecordingAction->setEnabled(true);
	closeRecordingButton->setEnabled(true);

	/* Enable the "Start" button, if connection to the data server */
	if ((dataClient) && (dataClient->connected()))
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
	QDir saveDir = settings.getSaveDir();
	return saveDir.absolutePath() + "/" + name;
}

bool MealogWindow::isDefaultSaveFile(void) {
	return ( (fileLine->text() == 
				Mealog::DEFAULT_SAVE_FILE.fileName()) && 
			(settings.getSaveDir() == 
			 	Mealog::DEFAULT_SAVE_DIR.absolutePath()));
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

	/* Switch on array type and set configuration for HiDens */

	double adcRange = Mealog::ADC_RANGES.at(adcRangeBox->currentIndex());
	recording->setOffset(adcRange);
	recording->setGain((adcRange * 2) / (1 << 16));

	/* Date of recording */
	recording->setDate();
}

void MealogWindow::initDataServer(void) {
	dataClient->setLength(recording->length());
	dataClient->setAdcRange(recording->offset());
	dataClient->setTrigger(triggerBox->currentText());
	/*
	if (array == "hidens")
		dataClient->setConfiguration(hdConfigBox->currentText());
	*/
	dataClient->initExperiment();
}

bool MealogWindow::confirmFileOverwrite(const QFile &path) {
	QMessageBox box(this);
	box.setWindowTitle("File exists"); // ignore on OSX
	box.setText("The selected file already exists.");
	box.setInformativeText("The file \"" + 
			path.fileName() + "\" already exists. Overwrite?");
	box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	box.setDefaultButton(QMessageBox::Cancel);
	return (box.exec() == QMessageBox::Ok);
}

void MealogWindow::setParameterSelectionsEnabled(bool enabled) {
	adcRangeBox->setEnabled(enabled);
	triggerBox->setEnabled(enabled);
	totalTimeLine->setReadOnly(!enabled);
	fileLine->setEnabled(enabled);
	choosePathButton->setEnabled(enabled);
}

void MealogWindow::closeRecordingWithCheck(void) {
	if ((recordingStatus & Mealog::STARTED) && 
			(recordingStatus & Mealog::RECORDING)) {
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
		}
		//if (recordingStatus & Mealog::NOT_STARTED)
			//QFile::remove(getFullFilename());
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
	setServerInterfaceEnabled(true);
	newRecordingButton->setEnabled(true);
	newRecordingAction->setEnabled(true);
	plotWindow->unblockResize();

	/* Disconnect data client, if this is a recording */
	if ((dataClient) && (dataClient->connected())) {
		disconnect(dataClient, &dataclient::DataClient::disconnected,
				this, &MealogWindow::handleServerDisconnection);
		disconnect(dataClient, &dataclient::DataClient::error,
				this, &MealogWindow::handleServerError);
		disconnect(connectToServerButton, &QPushButton::clicked,
				this, &MealogWindow::disconnectFromDataServer);
		dataClient->disconnect();
		dataClient->deleteLater();
		connect(connectToServerButton, &QPushButton::clicked,
				this, &MealogWindow::connectToDataServer);
		connect(connectToServerButton, &QPushButton::clicked,
				this, &MealogWindow::connectToDataServer);
	}
	serverStatus->setText("Not connected");
	connectToServerButton->setText("Connect");
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
		recording = new H5Rec::H5Recording(filename.toStdString());
	} catch (std::invalid_argument &e) {
		QMessageBox::critical(this, "Could not open recording",
				"An error occurred opening the recording file:\n\n" + 
				filename + "\n\nPlease verify it is a valid recording.");
		return;
	}
	fileLine->setText(filename.section("/", -1));
	QString path = filename.section("/", 0, -2);
	settings.setSaveFilename(fileLine->text());
	recordingStatus = (
			Mealog::INITIALIZED |
			Mealog::NOT_STARTED |
			Mealog::PLAYBACK
		);
	initPlayback();
	settings.setSaveDir(path);
	setServerInterfaceEnabled(false);
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

void MealogWindow::setServerInterfaceEnabled(bool enabled) {
	connectToServerButton->setEnabled(enabled);
	serverHost->setEnabled(enabled);
	if (!enabled)
		serverStatus->setText("Disabled");
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
	//serverStatus->setText("Connection to NI-DAQ server ended");
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
	connect(connectToServerButton, &QPushButton::clicked,
			this, &MealogWindow::connectToDataServer);
	connect(startButton, &QPushButton::clicked,
			this, &MealogWindow::startRecording);
	connect(showPlotWindow, &QAction::triggered,
			plotWindow, &PlotWindow::toggleVisible);
	connect(showControlsWindow, &QAction::toggled,
			this, &MealogWindow::setVisible);
	connect(showOAWindow, &QAction::toggled,
			oawindow, &OAWindow::toggleVisible);
	connect(oawindow, &OAWindow::setRunning,
			this, &MealogWindow::setOnlineAnalysisRunning);
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

void MealogWindow::chooseHidensConfiguration()
{
	QMessageBox::warning(this, "Not implemented",
			"Choosing HiDens configurations is not yet supported.");
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

void MealogWindow::connectToDataServer(void) {
	statusBar->showMessage("Connecting to data server");
	serverStatus->setText("Connecting ...");
	if (array == "hidens") 
		dataClient = new hdclient::HidensClient(serverHost->text());
	else
		dataClient = new mcsclient::McsClient(serverHost->text());
	connect(dataClient, &dataclient::DataClient::connectionMade,
			this, &MealogWindow::handleDataServerConnection);
	disconnect(connectToServerButton, &QPushButton::clicked,
			this, &MealogWindow::connectToDataServer);
	dataClient->connect();
	setServerInterfaceEnabled(false);
}

void MealogWindow::handleServerDisconnection(void) {
	setParameterSelectionsEnabled(true);
	setPlaybackButtonsEnabled(false);
	setPlaybackMovementButtonsEnabled(false);
	disconnectFromDataServer();
	closeRecording();
}

void MealogWindow::handleServerError(void) {
	connectToServerButton->setText("Connect");
	serverStatus->setText("Connection to server interrupted");
	if (recordingStatus & Mealog::STARTED) {
		disconnect(dataClient, &dataclient::DataClient::dataAvailable,
				this, &MealogWindow::recvData);
		closeRecording();
		QMessageBox::critical(this, "Connection interrupted",
				"Connection to data server application interrupted."\
				" Recording has been stopped, with all previous data "\
				"written to disk.");
	}
	//setServerInterfaceEnabled(true);
	//setPlaybackButtonsEnabled(false);
	//setPlaybackMovementButtonsEnabled(false);
	//setParameterSelectionsEnabled(true);
	//startButton->setText("Start");
	//statusBar->showMessage("Ready");
	//disconnect(connectToServerButton, &QPushButton::clicked,
			//this, &MealogWindow::disconnectFromDataServer);
	//connect(connectToServerButton, &QPushButton::clicked,
			//this, &MealogWindow::connectToDataServer);
	//disconnect(dataClient, &DaqClient::DaqClient::disconnected,
			//this, &MealogWindow::handleServerDisconnection);
	//disconnect(dataClient, &DaqClient::DaqClient::error,
			//this, &MealogWindow::handleServerError);
	//delete dataClient;
}

void MealogWindow::disconnectFromDataServer(void) {
	if (recordingStatus & Mealog::STARTED) {
		if (confirmCloseRecording() == QMessageBox::Cancel)
			return;
	}
	connectToServerButton->setText("Connect");
	serverStatus->setText("Not connected");
	setServerInterfaceEnabled(true);
	statusBar->showMessage("Ready");
	disconnect(connectToServerButton, &QPushButton::clicked,
			this, &MealogWindow::disconnectFromDataServer);
	connect(connectToServerButton, &QPushButton::clicked,
			this, &MealogWindow::connectToDataServer);
	disconnect(dataClient, &dataclient::DataClient::disconnected,
			this, &MealogWindow::handleServerDisconnection);
	disconnect(dataClient, &dataclient::DataClient::error,
			this, &MealogWindow::handleServerError);
	dataClient->disconnect();
	closeRecording();
}

void MealogWindow::handleDataServerConnection(bool made) {
	if (made) {
		QString tmp = (array == "hidens") ? "HiDens" : "MCS";
		serverStatus->setText(QString("Connected to %1 data server").arg(tmp));
		connectToServerButton->setText("Disconnect");
		connectToServerButton->setEnabled(true);
		serverHost->setEnabled(false);
		connect(connectToServerButton, &QPushButton::clicked,
				this, &MealogWindow::disconnectFromDataServer);
		connect(dataClient, &dataclient::DataClient::disconnected,
				this, &MealogWindow::handleServerDisconnection);
		connect(dataClient, &dataclient::DataClient::error,
				this, &MealogWindow::handleServerError);
		if (recordingStatus & Mealog::INITIALIZED) {
			initDataServer();
			startButton->setEnabled(true);
		}
		statusBar->showMessage("Ready");
	} else {
		disconnect(connectToServerButton, &QPushButton::clicked,
				this, &MealogWindow::disconnectFromDataServer);
		connect(connectToServerButton, &QPushButton::clicked,
				this, &MealogWindow::connectToDataServer);
		serverStatus->setText("Error connecting to data server, correct IP?");
		delete dataClient;
		setServerInterfaceEnabled(true);
		statusBar->showMessage("Ready");
	}
}

void MealogWindow::startRecording(void) {
	if (recordingStatus & Mealog::RECORDING) {
		connect(dataClient, &dataclient::DataClient::dataAvailable,
				this, &MealogWindow::recvData);
		dataClient->startRecording();
		statusBar->showMessage(QString("Recording data to %1").arg(
					getFullFilename()));
		recordingStatus = (
				Mealog::INITIALIZED | 
				Mealog::STARTED | 
				Mealog::RECORDING |
				Mealog::PLAYING | 
				dataSource
			);
	} else {
		playbackTimer->start();
		statusBar->showMessage(QString("Playing back data from %1").arg(
					getFullFilename()));
		recordingStatus = (
				Mealog::INITIALIZED | 
				Mealog::STARTED | 
				Mealog::PLAYBACK | 
				Mealog::PLAYING | 
				dataSource
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
	plotWindow->blockResize();
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
	plotWindow->unblockResize();
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
	plotWindow->blockResize();
	startButton->setShortcut(Qt::Key_Space);
	uint16_t oldStatus = recordingStatus & ~Mealog::PAUSED;
	recordingStatus = oldStatus | Mealog::PLAYING;
}

void MealogWindow::recvData(qint64 nsamples) {
	H5Rec::Samples samples(nsamples, dataClient->nchannels());

	/* If more than one block is available, receive each one in turn.
	 * Must explicitly receive each block, however, because data will
	 * not be contiguous in resulting Armadillo matrix, which uses
	 * column-major ordering.
	 */
	if (nsamples > dataClient->blockSize()) {
		auto blockSize = dataClient->blockSize();
		auto nblocks = nsamples / dataClient->blockSize();
		H5Rec::Samples tmp(blockSize, dataClient->nchannels());
		for (auto block = 0; block < nblocks; block++) {
			dataClient->recvData(blockSize, tmp.memptr());
			samples.rows(block * blockSize, ((block + 1) * blockSize) - 1) = tmp;
		}
	} else {
		dataClient->recvData(nsamples, samples.memptr());
	}

	/* Sign-invert new data, so spikes are upwards deflections */
	if (recordingStatus &= Mealog::RECORDING)
		samples *= -1;
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
	emit newDataPlotted(start, end);
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
	//if (lastSamplePlotted == recording->nsamples()) {
	if (lastSamplePlotted >= recording->lastValidSample()) {
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
	size_t numSamplesPerPlotBlock = ((settings.getJump() / 1000) *
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
	size_t numSamplesPerPlotBlock = ((settings.getJump() / 1000) *
			H5Rec::SAMPLE_RATE);
	if (lastSamplePlotted > numSamplesPerPlotBlock) {
		lastSamplePlotted -= 2 * numSamplesPerPlotBlock;
		plotDataBlock(lastSamplePlotted,
				lastSamplePlotted + numSamplesPerPlotBlock);
	}
}

void MealogWindow::jumpToBeginning(void) {
	size_t numSamplesPerPlotBlock = ((settings.getJump() / 1000) *
			H5Rec::SAMPLE_RATE);
	lastSamplePlotted = 0;
	plotDataBlock(lastSamplePlotted, lastSamplePlotted + numSamplesPerPlotBlock);
}

void MealogWindow::jumpToEnd(void) {
	size_t numSamplesPerPlotBlock = ((settings.getJump() / 1000) *
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
	settings.setDisplayScale(text.toFloat());
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

void MealogWindow::setOnlineAnalysisRunning(bool running)
{
	if (running)
		connect(this, &MealogWindow::newDataPlotted, 
				this, &MealogWindow::sendDataToOAWindow);
	else
		disconnect(this, &MealogWindow::newDataPlotted, 
				this, &MealogWindow::sendDataToOAWindow);
}

void MealogWindow::sendDataToOAWindow(uint64_t start, uint64_t end)
{
	oawindow->runAnalysis(start, recording->sampleRate(), 
			recording->data(start, end, oawindow->oaChannel()));
}

