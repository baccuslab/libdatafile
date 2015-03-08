/* ctrlwindow.cpp
 * --------------
 * Implementation of the main control interface in the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "ctrlwindow.h"

CtrlWindow::CtrlWindow(QWidget *parent) : QMainWindow(parent) {
	setWindowTitle("Meaview: Controls");
	setGeometry(PLOT_WINDOW_WIDTH + 10, 0, CTRL_WINDOW_WIDTH, CTRL_WINDOW_HEIGHT);
	initSettings();
	initCtrlWindowUI();
	initPlotWindow();
	initMenuBar();
	initStatusBar();
	initSignalsAndSlots();
}

CtrlWindow::~CtrlWindow() {
}

void CtrlWindow::initSettings() {
	settings.setSaveDir(DEFAULT_SAVE_DIR);
	settings.setSaveFilename(DEFAULT_SAVE_FILENAME);
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
	playbackTimer = new QTimer();
	playbackTimer->setInterval(settings.getRefreshInterval());
}

void CtrlWindow::initPlotWindow() {
	plotWindow = new PlotWindow(this);
	plotWindow->show();
}

void CtrlWindow::initMenuBar() {
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
	QAction *showPlotWindow = new QAction(tr("&Channel view"), this->windowsMenu);
	showPlotWindow->setShortcut(QKeySequence("Ctrl+0"));
	showPlotWindow->setCheckable(true);
	showPlotWindow->setChecked(true);
	connect(showPlotWindow, SIGNAL(triggered()), this->plotWindow, SLOT(toggleVisible()));
	this->windowsMenu->addAction(showPlotWindow);

	QAction *showControlsWindow = new QAction(tr("Control window"), this->windowsMenu);
	showControlsWindow->setShortcut(QKeySequence("Ctrl+1"));
	showControlsWindow->setCheckable(true);
	showControlsWindow->setChecked(true);
	connect(showControlsWindow, SIGNAL(triggered()), this, SLOT(toggleVisible()));
	this->windowsMenu->addAction(showControlsWindow);

	/* eventually same for online analysis and channel inspector */

	/* Add menus to bar and bar to PlotWindow */
	this->menubar->addMenu(this->fileMenu);
	this->menubar->addMenu(this->windowsMenu);
	this->setMenuBar(this->menubar);
}

void CtrlWindow::initCtrlWindowUI() {
	/* Set up GUI */
	mainLayout = new QGridLayout();

	/* Information group */
	infoGroup = new QGroupBox("Info");
	infoLayout = new QGridLayout();
	infoLayout->setColumnStretch(1, 3);
	filenameLabel = new QLabel("File:");
	filenameLine = new QLineEdit(settings.getSaveFilename());
	filenameLine->setToolTip("Name of current recording data file");
	filenameLine->setReadOnly(true);
	filenameValidator = new QRegExpValidator(QRegExp("(\\w+[-_]*)+"));
	filenameLine->setValidator(filenameValidator);
	savedirLabel = new QLabel("Save dir:");
	savedirLine = new QLineEdit(settings.getSaveDir());
	savedirLine->setToolTip("Directory of current recording data file");
	savedirLine->setReadOnly(true);
	chooseSavedirButton = new QPushButton("Choose");
	chooseSavedirButton->setToolTip("Choose save directory");
	timeLabel = new QLabel("Time:");
	timeLine = new QLineEdit("");
	timeLine->setToolTip("Current time in experiment");
	timeLineValidator = new QIntValidator(0, MAX_EXPERIMENT_LENGTH);
	timeLine->setValidator(timeLineValidator);
	infoLayout->addWidget(savedirLabel, 0, 0);
	infoLayout->addWidget(savedirLine, 0, 1, 1, 2);
	infoLayout->addWidget(chooseSavedirButton, 0, 3);
	infoLayout->addWidget(filenameLabel, 1, 0);
	infoLayout->addWidget(filenameLine, 1, 1);
	infoLayout->addWidget(timeLabel, 1, 2);
	infoLayout->addWidget(timeLine, 1, 3);
	infoGroup->setLayout(infoLayout);

	/* Playback group */
	playbackGroup = new QGroupBox("Playback");
	playbackLayout = new QGridLayout();
	restartButton = new QPushButton("Restart");
	restartButton->setToolTip("Jump to beginning of recording");
	restartButton->setEnabled(false);
	rewindButton = new QPushButton("Rewind");
	rewindButton->setToolTip("Rewind recording by amount given in 'Jump'");
	rewindButton->setEnabled(false);
	startPauseButton = new QPushButton("Start");
	startPauseButton->setToolTip("Start or pause recording playback. Data is still saved.");
	startPauseButton->setEnabled(false);
	stopButton = new QPushButton("Stop");
	stopButton->setToolTip("Stop playback and/or end recording");
	stopButton->setEnabled(false);
	forwardButton = new QPushButton("Forward");
	forwardButton->setToolTip("Move forward by amount given in 'Jump'");
	forwardButton->setEnabled(false);
	endButton = new QPushButton("End");
	endButton->setToolTip("Jump to end of recording");
	endButton->setEnabled(false);
	jumpLabel = new QLabel("Jump (ms):");
	jumpSpinBox = new QSpinBox();
	jumpSpinBox->setRange(JUMP_MIN, JUMP_MAX);
	jumpSpinBox->setSingleStep(JUMP_STEP_SIZE);
	jumpSpinBox->setReadOnly(true);
	jumpSpinBox->setValue(AIB_BLOCK_SIZE);
	refreshLabel = new QLabel("Refresh (ms):");
	refreshSpinBox = new QSpinBox();
	refreshSpinBox->setRange(MIN_REFRESH_INTERVAL, MAX_REFRESH_INTERVAL);
	refreshSpinBox->setValue(DISPLAY_REFRESH_INTERVAL);
	refreshSpinBox->setSingleStep(MIN_REFRESH_INTERVAL);
	refreshSpinBox->setReadOnly(true);
	playbackLayout->addWidget(rewindButton, 0, 0);
	playbackLayout->addWidget(startPauseButton, 0, 1);
	playbackLayout->addWidget(forwardButton, 0, 2);
	playbackLayout->addWidget(restartButton, 1, 0);
	playbackLayout->addWidget(stopButton, 1, 1);
	playbackLayout->addWidget(endButton, 1, 2);
	playbackLayout->addWidget(jumpLabel, 3, 0);
	playbackLayout->addWidget(jumpSpinBox, 3, 1);
	playbackLayout->addWidget(refreshLabel, 4, 0);
	playbackLayout->addWidget(refreshSpinBox, 4, 1);
	playbackGroup->setLayout(playbackLayout);

	/* Display parameters */
	displayGroup = new QGroupBox("Display");
	displayLayout = new QGridLayout();
	viewLabel = new QLabel("Channel view:");
	viewBox = new QComboBox();
	viewBox->setToolTip("Set arrangement of subplots to match the given array");
	for (auto &view : CHANNEL_VIEW_STRINGS)
		viewBox->addItem(view);
	viewBox->setCurrentIndex(
			viewBox->findText(settings.getChannelViewString()));
	colorLabel = new QLabel("Plot color:");
	colorBox = new QComboBox();
	colorBox->setToolTip("Set color of data plots");
	for (auto &color : PLOT_COLOR_STRINGS)
		colorBox->addItem(color);
	colorBox->setCurrentIndex(colorBox->findText(settings.getPlotColorString()));
	scaleLabel = new QLabel("Display scale:");
	scaleBox = new QComboBox();
	scaleBox->setToolTip("Change y-axis scaling of subplots. Larger means larger axis extent");
	for (auto &scale : DISPLAY_SCALES)
		scaleBox->addItem(QString::number(scale));
	scaleBox->setCurrentIndex(
			settings.getDisplayScales().indexOf(settings.getDisplayScale()));
	autoscaleLabel = new QLabel("Autoscale:");
	autoscaleBox = new QCheckBox();
	autoscaleBox->setToolTip("If checked, each subplot will scale to fit its data");
	autoscaleBox->setTristate(false);
	autoscaleBox->setChecked(false);
	autoMeanLabel = new QLabel("Automean:");
	autoMeanBox = new QCheckBox();
	autoMeanBox->setToolTip("If checked, mean-subtract each plot individually"\
			"\nThis may be very inefficient");
	autoMeanBox->setTristate(false);
	autoMeanBox->setChecked(false);
	displayLayout->addWidget(viewLabel, 0, 0, 1, 2);
	displayLayout->addWidget(viewBox, 0, 2, 1, 2);
	displayLayout->addWidget(colorLabel, 1, 0, 1, 2);
	displayLayout->addWidget(colorBox, 1, 2, 1, 2);
	displayLayout->addWidget(scaleLabel, 2, 0, 1, 2);
	displayLayout->addWidget(scaleBox, 2, 2, 1, 2);
	displayLayout->addWidget(autoscaleLabel, 3, 0);
	displayLayout->addWidget(autoscaleBox, 3, 1);
	displayLayout->addWidget(autoMeanLabel, 3, 2);
	displayLayout->addWidget(autoMeanBox, 3, 3);
	displayGroup->setLayout(displayLayout);

	/* Online analysis */
	onlineAnalysisGroup = new QGroupBox("Online analysis");
	onlineAnalysisLayout = new QGridLayout();
	noneAnalysisButton = new QRadioButton("None");
	noneAnalysisButton->setToolTip("Do not compute any online analysis");
	noneAnalysisButton->setChecked(true);
	temporalAnalysisButton = new QRadioButton("Temporal kernel");
	temporalAnalysisButton->setEnabled(false);
	temporalAnalysisButton->setToolTip(
			"This computes a 1D kernel of a purely temporal stimulus");
	linesAnalysisButton = new QRadioButton("Lines kernel");
	linesAnalysisButton->setEnabled(false);
	linesAnalysisButton->setToolTip("This computes a kernel with one temporal and" \
			"\none spatial dimension, e.g., white noise lines");
	spatiotemporalAnalysisButton = new QRadioButton("Spatiotemporal kernel");
	spatiotemporalAnalysisButton->setEnabled(false); // until we load a stimulus 
	spatiotemporalAnalysisButton->setToolTip(
			"Compute a full 2D spatiotemporal kernel, e.g., from white noise checkers");
	loadStimulusButton = new QPushButton("Load stimulus");
	loadStimulusButton->setToolTip("Choose a stimulus from which to compute the kernel");
	lengthLabel = new QLabel("Length:");
	lengthSpinBox = new QSpinBox();
	lengthSpinBox->setToolTip(
			"Set the number of time points computed in the online analysis");
	lengthSpinBox->setRange(
			ONLINE_ANALYSIS_MIN_LENGTH, ONLINE_ANALYSIS_MAX_LENGTH);
	lengthSpinBox->setValue(settings.getOnlineAnalysisLength());
	showAnalysisWindowButton = new QPushButton("Show analysis");
	showAnalysisWindowButton->setToolTip(
			"Open the window showing the running results of the online analysis");
	targetChannelLabel = new QLabel("Target channel:");
	targetChannelLine = new QLineEdit("0");
	targetChannelValidator = new QIntValidator(0, NUM_CHANNELS - 1);
	targetChannelLine->setToolTip("Choose the channel whose kernel should be computed");
	onlineAnalysisLayout->addWidget(noneAnalysisButton, 0, 0);
	onlineAnalysisLayout->addWidget(lengthLabel, 0, 1);
	onlineAnalysisLayout->addWidget(lengthSpinBox, 0, 2);
	onlineAnalysisLayout->addWidget(targetChannelLabel, 1, 1);
	onlineAnalysisLayout->addWidget(targetChannelLine, 1, 2);
	onlineAnalysisLayout->addWidget(temporalAnalysisButton, 1, 0);
	onlineAnalysisLayout->addWidget(linesAnalysisButton, 2, 0);
	onlineAnalysisLayout->addWidget(spatiotemporalAnalysisButton, 3, 0);
	onlineAnalysisLayout->addWidget(loadStimulusButton, 4, 0);
	onlineAnalysisLayout->addWidget(showAnalysisWindowButton, 4, 2);
	onlineAnalysisGroup->setLayout(onlineAnalysisLayout);

	/* Nidaq... */
	nidaqGroup = new QGroupBox("NI-DAQ");
	nidaqLayout = new QGridLayout();

	/* Add all to window */
	mainLayout->addWidget(infoGroup, 0, 0);
	mainLayout->addWidget(playbackGroup, 1, 0);
	mainLayout->addWidget(displayGroup, 2, 0);
	mainLayout->addWidget(onlineAnalysisGroup, 3, 0);
	this->setCentralWidget(new QWidget(this));
	centralWidget()->setLayout(mainLayout);
}

void CtrlWindow::initStatusBar() {
	this->statusBar = new QStatusBar();
	statusLabel = new QLabel("Ready");
	this->statusBar->addWidget(statusLabel);
	this->setStatusBar(this->statusBar);
}

void CtrlWindow::togglePlayback() {
	if (this->isPlaying) {
		this->playbackTimer->stop();
		this->startPauseButton->setText("Start");
		this->startPauseButton->setStyleSheet("QPushButton {color : black}");
	} else {
		this->playbackTimer->start();
		this->startPauseButton->setText("Stop");
		this->startPauseButton->setStyleSheet("QPushButton {color : rgb(178, 51, 51)}");
	}
	this->isPlaying = !this->isPlaying;
}

void CtrlWindow::loadRecording() {
	statusLabel->setText("Loading recording");
	QString filename = QFileDialog::getOpenFileName(
			this, tr("Load recording"),
			DEFAULT_SAVE_DIR, tr("Recordings (*.bin)"));
	if (filename.isNull())
		return;
	
	/* Open the recording */
	recording = new PlaybackRecording(filename);
	this->plotWindow->recording = recording;
	initPlaybackRecording();
	statusLabel->setText("Ready");
}

void CtrlWindow::initPlaybackRecording() {
	startPauseButton->setEnabled(true);
	timeLine->setText(QString::number(
				this->recording->getFile().getNumSamples() / SAMPLE_RATE));
	timeLine->setReadOnly(true);
	QFileInfo finfo(this->recording->getFile().getFilename());
	savedirLine->setText(finfo.dir().absolutePath());
	filenameLine->setText(finfo.baseName()); 
	connect(startPauseButton, SIGNAL(clicked()), this, SLOT(togglePlayback()));
	//connect(playbackTimer, SIGNAL(timeout()), this->plotWindow, SLOT(plotNextDataBlock()));
	connect(this->plotWindow->getChannelPlot(), SIGNAL(afterReplot()), 
			this, SLOT(updateTimeLine()));
}

void CtrlWindow::openNewRecording() {
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

void CtrlWindow::initSignalsAndSlots() {
	connect(this->playbackTimer, SIGNAL(timeout()),
			this->plotWindow, SLOT(plotNextDataBlock()));
	connect(this->filenameLine, SIGNAL(editingFinished()), 
			this, SLOT(updateFilename()));
	connect(this->chooseSavedirButton, SIGNAL(clicked()), 
			this, SLOT(chooseSaveDir()));
	connect(this->timeLine, SIGNAL(editingFinished()), 
			this, SLOT(updateTime()));
	connect(this->jumpSpinBox, SIGNAL(valueChanged(int)),
			this, SLOT(updateJump(int)));
	connect(this->viewBox, SIGNAL(currentIndexChanged(QString)), 
			this, SLOT(updateView(QString)));
	connect(this->colorBox, SIGNAL(currentIndexChanged(QString)),
			this, SLOT(updateColor(QString)));
	connect(this->scaleBox, SIGNAL(currentIndexChanged(QString)),
			this, SLOT(updateScale(QString)));
	connect(this->autoscaleBox, SIGNAL(stateChanged(int)), 
			this, SLOT(updateAutoscale(int)));
	connect(this->chooseSavedirButton, SIGNAL(clicked()),
			this, SLOT(openNewRecordingWindow()));
	connect(this->targetChannelLine, SIGNAL(editingFinished()),
			this, SLOT(setOnlineAnalysisTargetChannel()));
	connect(this->refreshSpinBox, SIGNAL(valueChanged(int)),
			this, SLOT(updateRefreshInterval(int)));
	connect(this->autoMeanBox, SIGNAL(stateChanged(int)),
			this, SLOT(updateAutoMean(int)));
	connect(this->plotWindow->getChannelPlot(), SIGNAL(subplotDoubleClicked(int)),
			this, SLOT(openChannelInspectWindow(int)));
	connect(this->recording, SIGNAL(endOfPlaybackFile()),
			this->playbackTimer, SLOT(stop()));
}

/* SIGNALS AND SLOTS */

void CtrlWindow::updateTimeLine() {
	int block = this->recording->getBlock();
	this->timeLine->setText(QString("%1 - %2 / %3").arg(
				(block / (AIB_BLOCK_SIZE / SAMPLE_RATE)) - 1).arg(
				block / (AIB_BLOCK_SIZE / SAMPLE_RATE)).arg(
				this->recording->getRecordingLength()));
}

void CtrlWindow::updateAutoMean(int checked) {
	this->settings.setAutoMean(checked == Qt::Checked);
}

void CtrlWindow::updateRefreshInterval(int i) {
	this->settings.setRefreshInterval(i);
}

void CtrlWindow::updateFilename() {
	this->settings.setSaveFilename(this->filenameLine->text());
}

void CtrlWindow::chooseSaveDir() {
	QFileDialog dialog(this, "Choose save directory", DEFAULT_SAVE_DIR);
	dialog.setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dialog.exec() == QDialog::Rejected)
		return;
	QString path = dialog.directory().absolutePath();
	this->settings.setSaveDir(path);
	this->savedirLine->setText(path);
}

void CtrlWindow::updateTime() {
	this->settings.setExperimentLength(this->timeLine->text().toUInt());
}

void CtrlWindow::updateView(QString view) {
	this->settings.setChannelView(view);
}

void CtrlWindow::updateScale(QString num) {
	this->settings.setDisplayScale(num.toFloat());
}

void CtrlWindow::updateJump(int jump) {
	this->settings.setJump(jump);
}

void CtrlWindow::updateColor(QString color) {
	this->settings.setPlotColor(color);
}

void CtrlWindow::updateAutoscale(int state) {
	bool checked = (state == Qt::Checked);
	autoscaleBox->setChecked(checked);
	scaleBox->setEnabled(!checked);
	settings.setAutoscale(checked);
}

void CtrlWindow::setOnlineAnalysisTargetChannel() {
	this->targetChannel = this->targetChannelLine->text().toUInt();
}

void CtrlWindow::toggleVisible() {
	this->setVisible(!this->isVisible());
}

void CtrlWindow::openChannelInspectWindow(int index) {
	ChannelInspectWindow *w = new ChannelInspectWindow(
			this->plotWindow->getChannelPlot(), index, this);
	//connect(this->playbackTimer, SIGNAL(timeout()), 
			//w, SLOT(replot()));
	w->show();
}

