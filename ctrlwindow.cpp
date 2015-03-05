/* ctrlwindow.cpp
 * Implementation of the main control interface in the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "ctrlwindow.h"

CtrlWindow::CtrlWindow(QWidget *parent) : QWidget(parent, Qt::Window) {
	this->setWindowTitle("Meaview: Controls");
	QRect rect = parent->geometry();
	this->setGeometry(rect.right() + 10, rect.top(), 
			CTRL_WINDOW_WIDTH, CTRL_WINDOW_HEIGHT);

	this->initUI();
	this->initSignalsAndSlots();
}

CtrlWindow::~CtrlWindow() {
}

void CtrlWindow::initUI() {
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
	infoLayout->addWidget(savedirLine, 0, 1);
	infoLayout->addWidget(chooseSavedirButton, 0, 3);
	infoLayout->addWidget(filenameLabel, 1, 0);
	infoLayout->addWidget(filenameLine, 1, 1);
	infoLayout->addWidget(timeLabel, 2, 0);
	infoLayout->addWidget(timeLine, 2, 1);
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
	jumpSpinBox->setValue(AIB_BLOCK_SIZE);
	playbackLayout->addWidget(restartButton, 0, 0);
	playbackLayout->addWidget(rewindButton, 1, 0);
	playbackLayout->addWidget(startPauseButton, 0, 1);
	playbackLayout->addWidget(stopButton, 1, 1);
	playbackLayout->addWidget(forwardButton, 2, 0);
	playbackLayout->addWidget(endButton, 2, 1);
	playbackLayout->addWidget(jumpLabel, 3, 0);
	playbackLayout->addWidget(jumpSpinBox, 3, 1);
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
	displayLayout->addWidget(viewLabel, 0, 0);
	displayLayout->addWidget(viewBox, 0, 1);
	displayLayout->addWidget(colorLabel, 1, 0);
	displayLayout->addWidget(colorBox, 1, 1);
	displayLayout->addWidget(scaleLabel, 2, 0);
	displayLayout->addWidget(scaleBox, 2, 1);
	displayLayout->addWidget(autoscaleLabel, 3, 0);
	displayLayout->addWidget(autoscaleBox, 3, 1);
	displayGroup->setLayout(displayLayout);

	/* Online analysis */
	onlineAnalysisGroup = new QGroupBox("Online analysis");
	onlineAnalysisLayout = new QGridLayout();
	noneAnalysisButton = new QRadioButton("None");
	noneAnalysisButton->setToolTip("Do not compute any online analysis");
	noneAnalysisButton->setChecked(true);
	temporalAnalysisButton = new QRadioButton("Temporal kernel");
	temporalAnalysisButton->setToolTip("This computes a 1D kernel of a purely temporal stimulus");
	linesAnalysisButton = new QRadioButton("Lines kernel");
	linesAnalysisButton->setToolTip("This computes a kernel with one temporal and \none spatial dimension, e.g., white noise lines");
	spatiotemporalAnalysisButton = new QRadioButton("Spatiotemporal kernel");
	spatiotemporalAnalysisButton->setToolTip("Compute a full 2D spatiotemporal kernel, e.g., from white noise checkers");
	loadStimulusButton = new QPushButton("Load stimulus");
	loadStimulusButton->setToolTip("Choose a stimulus from which to compute the kernel");
	lengthLabel = new QLabel("Length:");
	lengthSpinBox = new QSpinBox();
	lengthSpinBox->setToolTip("Set the number of time points computed in the online analysis");
	lengthSpinBox->setRange(
			ONLINE_ANALYSIS_MIN_LENGTH, ONLINE_ANALYSIS_MAX_LENGTH);
	lengthSpinBox->setValue(settings.getOnlineAnalysisLength());
	onlineAnalysisLayout->addWidget(noneAnalysisButton, 0, 0);
	onlineAnalysisLayout->addWidget(temporalAnalysisButton, 1, 0);
	onlineAnalysisLayout->addWidget(linesAnalysisButton, 2, 0);
	onlineAnalysisLayout->addWidget(spatiotemporalAnalysisButton, 3, 0);
	onlineAnalysisLayout->addWidget(loadStimulusButton, 4, 0);
	onlineAnalysisLayout->addWidget(lengthLabel, 4, 1);
	onlineAnalysisLayout->addWidget(lengthSpinBox, 4, 2);
	onlineAnalysisGroup->setLayout(onlineAnalysisLayout);

	/* Nidaq... */
	nidaqGroup = new QGroupBox("NI-DAQ");
	nidaqLayout = new QGridLayout();

	/* Add all to window */
	mainLayout->addWidget(infoGroup, 0, 0);
	mainLayout->addWidget(playbackGroup, 1, 0);
	mainLayout->addWidget(displayGroup, 2, 0);
	mainLayout->addWidget(onlineAnalysisGroup, 3, 0);
	this->setLayout(mainLayout);
}

void CtrlWindow::initSignalsAndSlots() {
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
	this->autoscale = checked;
	this->autoscaleBox->setChecked(checked);
	this->scaleBox->setEnabled(!checked);
	this->settings.setAutoscale(checked);
}

