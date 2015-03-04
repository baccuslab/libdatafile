/* windows.cpp
 * Implementation of various windows used in the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

/* C++ includes */

/* Qt includes */
#include <QMainWindow>
#include <QGridLayout>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QDialog>
#include <QSettings>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QDebug>
#include <QtConcurrent>

/* meaview includes */
#include "windows.h"

/***************************************************
 **************** SettingsWindow *******************
 ***************************************************/
SettingsWindow::SettingsWindow(QWidget *parent) : QDialog(parent) {
	//settings = Settings();
	
	displayGroup = new QGroupBox("Display");
	displayLayout = new QGridLayout();
	viewLabel = new QLabel("Channel view:");
	viewBox = new QComboBox();
	for (auto &view : CHANNEL_VIEW_STRINGS)
		viewBox->addItem(view);
	viewBox->setCurrentIndex(
			CHANNEL_VIEW_STRINGS.indexOf(settings.getChannelViewString()));
	scaleLabel = new QLabel("Display scale:");
	scaleBox = new QComboBox();
	for (auto &scale : DISPLAY_SCALES)
		scaleBox->addItem(QString::number(scale));
	scaleBox->setCurrentIndex(
			DISPLAY_SCALES.indexOf(settings.getDisplayScale()));
	connect(scaleBox, SIGNAL(currentIndexChanged(int)), this->parent(), SLOT(setScale(int)));
	autoscaleBox = new QCheckBox("Autoscale");
	autoscaleBox->setTristate(false);
	autoscaleBox->setChecked(this->settings.getAutoscale());
	connect(autoscaleBox, SIGNAL(stateChanged(int)), this->parent(), SLOT(setAutoscale(int)));
	penColorLabel = new QLabel("Plot color:");
	penColorBox = new QComboBox();
	for (auto &color : PLOT_COLOR_STRINGS)
		penColorBox->addItem(color);
	penColorBox->setCurrentIndex(
			PLOT_COLOR_STRINGS.indexOf(settings.getPlotColorString()));
	displayLayout->addWidget(viewLabel, 0, 0);
	displayLayout->addWidget(viewBox, 0, 1);
	displayLayout->addWidget(scaleLabel, 1, 0);
	displayLayout->addWidget(scaleBox, 1, 1);
	displayLayout->addWidget(autoscaleBox, 1, 2);
	displayLayout->addWidget(penColorLabel, 2, 0);
	displayLayout->addWidget(penColorBox, 2, 1);
	displayGroup->setLayout(displayLayout);

	playbackGroup = new QGroupBox("Playback");
	playbackLayout = new QGridLayout();
	refreshLabel = new QLabel("Refresh:");
	refreshLine = new QLineEdit(QString::number(settings.getRefreshInterval()));
	refreshValidator = new QIntValidator(
			MIN_REFRESH_INTERVAL, MAX_REFRESH_INTERVAL);
	refreshLine->setValidator(refreshValidator);
	playbackLayout->addWidget(refreshLabel, 0, 0);
	playbackLayout->addWidget(refreshLine, 0, 1);
	playbackGroup->setLayout(playbackLayout);

	okGroup = new QGroupBox();
	okGroup->setFlat(true);
	okButton = new QPushButton("OK");
	connect(okButton, SIGNAL(clicked()), this, SLOT(applySettings()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	applyButton = new QPushButton("Apply");
	connect(applyButton, SIGNAL(clicked()), this, SLOT(applySettings()));
	cancelButton = new QPushButton("Cancel");
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	okLayout = new QHBoxLayout();
	okLayout->addWidget(okButton);
	okLayout->addWidget(applyButton);
	okLayout->addWidget(cancelButton);
	okGroup->setLayout(okLayout);

	mainLayout = new QGridLayout();
	mainLayout->addWidget(displayGroup, 0, 0);
	mainLayout->addWidget(playbackGroup, 0, 1);
	mainLayout->addWidget(okGroup, 1, 0);
	this->setLayout(mainLayout);
}

SettingsWindow::~SettingsWindow() {
}

void SettingsWindow::applySettings() {
	//qDebug() << this->objectName();
	this->settings.setChannelView(this->viewBox->currentText());
	this->settings.setRefreshInterval(this->refreshLine->text().toUInt());
	this->settings.setDisplayScale(this->scaleBox->currentText().toDouble());
	this->settings.setPlotColor(this->penColorBox->currentText());
	this->settings.setAutoscale(this->autoscaleBox->checkState() == Qt::Checked);
}


/***************************************************
 ************** NewRecordingWindow *****************
 ***************************************************/
NewRecordingWindow::NewRecordingWindow(QWidget *parent) : QDialog(parent) {

	/* Selection for plot arrangement */
	viewGroup = new QGroupBox("Channel view");
	viewLayout = new QVBoxLayout();
	viewBox = new QComboBox(this);
	for (auto &view : CHANNEL_VIEW_STRINGS)
		viewBox->addItem(view);
	viewBox->setCurrentIndex(viewBox->findText(DEFAULT_VIEW));
	connect(viewBox, SIGNAL(activated()), this, SLOT(setView()));
	viewLayout->addWidget(viewBox);
	viewGroup->setLayout(viewLayout);

	/* Select save directory */
	saveGroup = new QGroupBox("Save directory");
	saveLine = new QLineEdit(DEFAULT_SAVE_DIR);
	saveLine->setReadOnly(true);
	browseButton = new QPushButton("Browse");
	connect(browseButton, SIGNAL(clicked()), this, SLOT(chooseDirectory()));
	saveLayout = new QGridLayout();
	saveLayout->addWidget(saveLine, 0, 0);
	saveLayout->addWidget(browseButton, 0, 1);
	saveGroup->setLayout(saveLayout);

	/* Select filename */
	fileGroup = new QGroupBox(tr("&Filename"));
	fileLine = new QLineEdit(DEFAULT_SAVE_FILENAME);
	fileValidator = new QRegExpValidator(QRegExp("(\\w+[-_]*)+"));
	fileLine->setValidator(fileValidator);
	fileLayout = new QVBoxLayout();
	fileLayout->addWidget(fileLine);
	fileGroup->setLayout(fileLayout);

	/* Pick a length of the recording */
	timeGroup = new QGroupBox("Length of recording");
	timeValidator = new QIntValidator(0, settings.getExperimentLength());
	timeLine = new QLineEdit(QString::number(settings.getExperimentLength()));
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
	return this->settings.getSaveDir();
}

QString NewRecordingWindow::getSaveFilename() {
	return this->settings.getSaveFilename();
}

QString NewRecordingWindow::getView() {
	return this->settings.getChannelViewString();
}

uint NewRecordingWindow::getTime() {
	return this->settings.getExperimentLength();
}

QString NewRecordingWindow::getFullFilename() {
	QString s = this->getSaveDir();
	if (!s.endsWith("/"))
		s.append("/");
	s.append(this->getSaveFilename());
	return s.append(SAVE_FILE_EXTENSION);
}

void NewRecordingWindow::setView() {
	//QAction *sender = dynamic_cast<QAction *>(QObject::sender());
	//this->viewButton->setText(sender->text());
	QComboBox *sender = dynamic_cast<QComboBox *>(QObject::sender());
	this->settings.setChannelView(sender->currentText());
}

int NewRecordingWindow::validateChoices() {
	this->settings.setSaveDir(this->saveLine->text());
	this->settings.setSaveFilename(this->fileLine->text());
	this->settings.setChannelView(this->viewBox->currentText());
	this->settings.setExperimentLength(this->timeLine->text().toUInt());
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
	QFileDialog dialog(this, "Choose save directory", DEFAULT_SAVE_DIR);
	dialog.setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dialog.exec() == QDialog::Rejected)
		return;
	QString path = dialog.directory().absolutePath();
	this->settings.setSaveDir(path);
	this->saveLine->setText(path);
}

