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
	settings = new QSettings();

	displayGroup = new QGroupBox("Display");
	displayLayout = new QGridLayout();
	viewLabel = new QLabel("Channel view:");
	viewBox = new QComboBox();
	for (auto &view : VIEW_LABELS)
		viewBox->addItem(view);
	viewBox->setCurrentIndex(
			VIEW_LABELS.indexOf(settings->value("view").toString()));
	scaleLabel = new QLabel("Display scale:");
	scaleBox = new QComboBox();
	for (auto &scale : DISPLAY_SCALES)
		scaleBox->addItem(QString::number(scale));
	scaleBox->setCurrentIndex(
			DISPLAY_SCALES.indexOf(settings->value("scale").toFloat()));
	penColorLabel = new QLabel("Plot color:");
	penColorBox = new QComboBox();
	for (auto &color : PLOT_COLOR_LABELS)
		penColorBox->addItem(color);
	penColorBox->setCurrentIndex(
			PLOT_COLOR_LABELS.indexOf(settings->value("pen-label").toString()));
	displayLayout->addWidget(viewLabel, 0, 0);
	displayLayout->addWidget(viewBox, 0, 1);
	displayLayout->addWidget(scaleLabel, 1, 0);
	displayLayout->addWidget(scaleBox, 1, 1);
	displayLayout->addWidget(penColorLabel, 2, 0);
	displayLayout->addWidget(penColorBox, 2, 1);
	displayGroup->setLayout(displayLayout);

	playbackGroup = new QGroupBox("Playback");
	playbackLayout = new QGridLayout();
	refreshLabel = new QLabel("Refresh:");
	refreshLine = new QLineEdit(settings->value("refresh").toString());
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
	this->settings->setValue("view", 
			QVariant(VIEW_LABELS.indexOf(this->viewBox->currentText())));
	this->settings->setValue("refresh",
			QVariant(this->refreshLine->text().toUInt()));
	this->settings->setValue("scale",
			QVariant(this->scaleBox->currentText().toDouble()));
	this->settings->setValue("pen",
			QVariant(PEN_MAP[this->penColorBox->currentText()]));
	this->settings->setValue("pen-label",
			QVariant(this->penColorBox->currentText()));
	//for (auto &key : this->settings->allKeys())
		//qDebug() << key << this->settings->value(key);
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
	return s.append(SAVE_FILE_EXTENSION);
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
	QFileDialog dialog(this, "Choose save directory", DEFAULT_SAVE_DIR);
	dialog.setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dialog.exec() == QDialog::Rejected)
		return;
	QString path = dialog.directory().absolutePath();
	this->settings->setValue("savedir", QVariant(path));
	this->saveLine->setText(path);
}

