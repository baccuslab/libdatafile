/* windows.cpp
 * Implementation of various windows used in the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

/* C++ includes */

/* Qt includes */
//#include <QMenu>
//#include <QToolBar>
//#include <QAction>
//#include <QLabel>
#include <QDebug>

/* meaview includes */
#include "windows.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
	this->setGeometry(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	this->setWindowTitle("Meaview: Channel view");
	initMenuBar();
	initStatusBar();
}

MainWindow::~MainWindow() {
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
	connect(newRecordingAction, SIGNAL(triggered()), this, SLOT(openNewRecording()));
	this->fileMenu->addAction(newRecordingAction);

	/* Load recording for replay */
	QAction *loadRecordingAction = new QAction(tr("&Load"), this->fileMenu);
	connect(loadRecordingAction, SIGNAL(triggered()), this, SLOT(loadRecording()));
	this->fileMenu->addAction(loadRecordingAction);

	/* Add menus to bar and bar to MainWindow */
	this->menubar->addMenu(this->fileMenu);
	this->setMenuBar(this->menubar);

	//QToolBar *bar = new QToolBar("toolbar");
	//this->addToolBar(Qt::TopToolBarArea, bar);
}

void MainWindow::initStatusBar() {
	this->statusBar = new QStatusBar();
	QLabel *statusLabel = new QLabel("Ready");
	this->statusBar->addWidget(statusLabel);
	this->setStatusBar(this->statusBar);
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
}

void MainWindow::loadRecording() {
}

NewRecordingWindow::NewRecordingWindow(QWidget *parent) : QDialog(parent) {
	/* Default choices */
	settings = new QSettings("baccuslab", "meaview");
	settings->setValue("savedir", QVariant(QString::fromStdString(DEFAULT_SAVE_DIR)));
	settings->setValue("filename", QVariant(QString::fromStdString(DEFAULT_SAVE_FILENAME)));
	settings->setValue("view", QVariant(QString::fromStdString(DEFAULT_VIEW)));

	/* Selection for plot arrangement */
	viewGroup = new QGroupBox("Channel view");
	viewLayout = new QVBoxLayout();
	viewButton = new QPushButton(QString::fromStdString(DEFAULT_VIEW));
	viewActionGroup = new QActionGroup(viewButton);
	viewMenu = new QMenu();
	for (auto &view : VIEW_LABELS) {
		QAction *act = new QAction(QString::fromStdString(view), viewActionGroup);
		act->setCheckable(true);
		if (view == DEFAULT_VIEW) {
			act->setChecked(true);
			viewMenu->setDefaultAction(act);
		} else {
			act->setChecked(false);
		}
		connect(act, SIGNAL(triggered()), this, SLOT(setView()));
		viewActionGroup->addAction(act);
		viewMenu->addAction(act);
	}
	viewButton->setMenu(viewMenu);
	viewLayout->addWidget(viewButton);
	viewGroup->setLayout(viewLayout);

	/* Select save directory */
	saveGroup = new QGroupBox("Save directory");
	saveLine = new QLineEdit(QString::fromStdString(DEFAULT_SAVE_DIR));
	saveLine->setReadOnly(true);
	browseButton = new QPushButton("Browse");
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

	/* OK/cancel buttons */
	buttonGroup = new QGroupBox();
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
	layout->addWidget(fileGroup, 0, 2, 1, 3);
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

QString NewRecordingWindow::getFullFilename() {
	QString s = this->getSaveDir();
	if (!s.endsWith("/"))
		s.append("/");
	return s.append(this->getSaveFilename());
}

void NewRecordingWindow::setView() {
	QAction *sender = dynamic_cast<QAction *>(QObject::sender());
	this->viewButton->setText(sender->text());
	this->settings->value("view", sender->text());
}

int NewRecordingWindow::validateChoices() {
	this->settings->setValue("savedir", QVariant(this->saveLine->text()));
	this->settings->setValue("filename", QVariant(this->fileLine->text()));
	this->settings->setValue("view", QVariant(this->viewButton->text()));
	QFileInfo finfo(this->saveLine->text());
	if ( ((finfo.permissions() & QFileDevice::ReadOwner) == 0) | 
		((finfo.permissions() & QFileDevice::WriteOwner) == 0) ) {
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
