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

ChannelInspectWindow::ChannelInspectWindow(
		ChannelPlot *p, int index, QWidget *parent) : QWidget(parent, Qt::Window) {
	/* Save parent data */
	channelPlot = p;
	this->index = index;

	/* Create a plot, axis, and graph */
	plot = new QCustomPlot(this);
	rect = new QCPAxisRect(plot);
	plot->plotLayout()->removeAt(0);
	plot->plotLayout()->addElement(0, 0, rect);
	graph = new QCPGraph(rect->axis(QCPAxis::atBottom),
			rect->axis(QCPAxis::atLeft));
	plot->addPlottable(graph);

	/* Style graph */
	graph->keyAxis()->setTicks(false);
	graph->keyAxis()->setTicks(false);
	graph->keyAxis()->setTickLabels(false);
	graph->keyAxis()->grid()->setVisible(false);
	graph->keyAxis()->setRange(0, SAMPLE_RATE * 
			this->settings.getRefreshInterval() / 1000);
	graph->valueAxis()->setTicks(false);
	graph->valueAxis()->setTickLabels(false);
	graph->valueAxis()->grid()->setVisible(false);
	graph->setPen(settings.getPlotPen());

	/* Assign the data from the requested subplot. Would like to avoid
	 * copying here, but not sure how to share data across plots (and threads)
	 * without segfault. Doesn't seem to hinder performance.
	 */
	QCPDataMap *dataMap = channelPlot->getSubplot(index)->data();
	graph->setData(dataMap, true);
	graph->rescaleValueAxis();
	plot->replot();

	/* Put plot in window and arrange window */
	layout = new QGridLayout();
	layout->addWidget(plot);
	this->setLayout(layout);
	this->setGeometry(PLOT_WINDOW_WIDTH + 10, CTRL_WINDOW_HEIGHT + 70, 
			INSPECTOR_WINDOW_WIDTH, INSPECTOR_WINDOW_HEIGHT);
	this->setWindowTitle(QString("Meaview: Channel %1").arg(index));
	this->setAttribute(Qt::WA_DeleteOnClose);

	connect(channelPlot, SIGNAL(afterReplot()),
			this, SLOT(replot()));
}

ChannelInspectWindow::~ChannelInspectWindow() {
}

void ChannelInspectWindow::replot() {
	qDebug() << "Data transfer start: " << QTime::currentTime();
	graph->setData(this->channelPlot->getSubplot(this->index)->data(), true);
	qDebug() << "Data transfer end: " << QTime::currentTime();
	graph->setPen(settings.getPlotPen());
	graph->rescaleValueAxis();
	this->plot->replot();
}

/***************************************************
 ************** NewRecordingWindow *****************
 ***************************************************/
//NewRecordingWindow::NewRecordingWindow(QWidget *parent) : QDialog(parent) {

	//[> Selection for plot arrangement <]
	//viewGroup = new QGroupBox("Channel view");
	//viewLayout = new QVBoxLayout();
	//viewBox = new QComboBox(this);
	//for (auto &view : CHANNEL_VIEW_STRINGS)
		//viewBox->addItem(view);
	//viewBox->setCurrentIndex(viewBox->findText(DEFAULT_VIEW));
	//connect(viewBox, SIGNAL(activated()), this, SLOT(setView()));
	//viewLayout->addWidget(viewBox);
	//viewGroup->setLayout(viewLayout);

	//[> Select save directory <]
	//saveGroup = new QGroupBox("Save directory");
	//saveLine = new QLineEdit(DEFAULT_SAVE_DIR);
	//saveLine->setReadOnly(true);
	//browseButton = new QPushButton("Browse");
	//connect(browseButton, SIGNAL(clicked()), this, SLOT(chooseDirectory()));
	//saveLayout = new QGridLayout();
	//saveLayout->addWidget(saveLine, 0, 0);
	//saveLayout->addWidget(browseButton, 0, 1);
	//saveGroup->setLayout(saveLayout);

	//[> Select filename <]
	//fileGroup = new QGroupBox(tr("&Filename"));
	//fileLine = new QLineEdit(DEFAULT_SAVE_FILENAME);
	//fileValidator = new QRegExpValidator(QRegExp("(\\w+[-_]*)+"));
	//fileLine->setValidator(fileValidator);
	//fileLayout = new QVBoxLayout();
	//fileLayout->addWidget(fileLine);
	//fileGroup->setLayout(fileLayout);

	//[> Pick a length of the recording <]
	//timeGroup = new QGroupBox("Length of recording");
	//timeValidator = new QIntValidator(0, settings.getExperimentLength());
	//timeLine = new QLineEdit(QString::number(settings.getExperimentLength()));
	//timeLine->setValidator(timeValidator);
	//timeLayout = new QVBoxLayout();
	//timeLayout->addWidget(timeLine);
	//timeGroup->setLayout(timeLayout);

	//[> OK/cancel buttons <]
	//buttonGroup = new QGroupBox();
	//buttonGroup->setFlat(true);
	//okButton = new QPushButton("OK");
	//connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	//okButton->setDefault(true);
	//cancelButton = new QPushButton("Cancel");
	//connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	//buttonLayout = new QHBoxLayout();
	//buttonLayout->addWidget(okButton);
	//buttonLayout->addWidget(cancelButton);
	//buttonGroup->setLayout(buttonLayout);

	//[> Layout <]
	//layout = new QGridLayout();
	//layout->addWidget(buttonGroup, 2, 0);
	//layout->addWidget(viewGroup, 0, 0, 1, 2);
	//layout->addWidget(fileGroup, 0, 2, 1, 2);
	//layout->addWidget(timeGroup, 0, 4, 1, 1);
	//layout->addWidget(saveGroup, 1, 0, 1, 5);

	//this->setLayout(layout);
	//this->setWindowTitle("Create new recording");

//}

//NewRecordingWindow::~NewRecordingWindow() {
//}

//QString NewRecordingWindow::getSaveDir() {
	//return this->settings.getSaveDir();
//}

//QString NewRecordingWindow::getSaveFilename() {
	//return this->settings.getSaveFilename();
//}

//QString NewRecordingWindow::getView() {
	//return this->settings.getChannelViewString();
//}

//uint NewRecordingWindow::getTime() {
	//return this->settings.getExperimentLength();
//}

//QString NewRecordingWindow::getFullFilename() {
	//QString s = this->getSaveDir();
	//if (!s.endsWith("/"))
		//s.append("/");
	//s.append(this->getSaveFilename());
	//return s.append(SAVE_FILE_EXTENSION);
//}

//void NewRecordingWindow::setView() {
	////QAction *sender = dynamic_cast<QAction *>(QObject::sender());
	////this->viewButton->setText(sender->text());
	//QComboBox *sender = dynamic_cast<QComboBox *>(QObject::sender());
	//this->settings.setChannelView(sender->currentText());
//}

//int NewRecordingWindow::validateChoices() {
	//this->settings.setSaveDir(this->saveLine->text());
	//this->settings.setSaveFilename(this->fileLine->text());
	//this->settings.setChannelView(this->viewBox->currentText());
	//this->settings.setExperimentLength(this->timeLine->text().toUInt());
	//QFileInfo finfo(this->saveLine->text());
	//if (!finfo.permission(QFileDevice::ReadOwner | QFileDevice::WriteOwner)) {
		//QMessageBox msg;
		//msg.setText("Permissions error");
		//msg.setInformativeText(QString(
				//"The current user does not have permissions for"
				//" the requested save directory:\n%1").arg(
				//this->saveLine->text()));
		//msg.setStandardButtons(QMessageBox::Ok);
		//msg.exec();
		//return -1;
	//}
	//return 0;
//}

//void NewRecordingWindow::chooseDirectory() {
	//QFileDialog dialog(this, "Choose save directory", DEFAULT_SAVE_DIR);
	//dialog.setOptions(QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	//if (dialog.exec() == QDialog::Rejected)
		//return;
	//QString path = dialog.directory().absolutePath();
	//this->settings.setSaveDir(path);
	//this->saveLine->setText(path);
//}

