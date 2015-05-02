/* mealogwindow.h
 * Header file for Mealog application's main window class.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _MEALOG_WINDOW_H_
#define _MEALOG_WINDOW_H_

/* Qt includes */
#include <QMainWindow>
#include <QGridLayout>
#include <QGroupBox>
#include <QStatusBar>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QIntValidator>
#include <QSpinBox>
#include <QCheckBox>
#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QFile>
#include <QDir>

/* Project library includes */
#include "daqclient/include/daqclient.h"
#include "h5recording/include/h5recording.h"
#include "messaging/logserver.pb.h"

/* Mealog includes */
#include "settings.h"
#include "plotwindow.h"

/* Settings */
namespace Mealog {

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 200;
const QFile DEFAULT_SAVE_FILE("default-data.h5");
const QDir DEFAULT_SAVE_DIR(QDir::homePath());
const unsigned int DEFAULT_EXPERIMENT_LENGTH = 1000;
const unsigned int MAX_EXPERIMENT_LENGTH = 10000;
const QList<double> ADC_RANGES = { 1, 2, 5, 10 };
const double DEFAULT_ADC_RANGE = 5;
const QStringList TRIGGERS = {"Photodiode", "None"};
const QString DEFAULT_NIDAQ_HOST = "127.0.0.1";
const QString IPC_HOST = "127.0.0.1";
const quint16 IPC_PORT = 44444;

enum INIT_STATUS:uint8_t {
	UNINITIALIZED = 0,
	INITIALIZED = 1
};

enum RECORDING_STATUS:uint8_t {
	NOT_STARTED = 2,
	STARTED = 4,
	FINISHED = 8,
	ERROR = 16
};

class MealogWindow;

};

class MealogWindow : public QMainWindow {
	Q_OBJECT

	public:
		MealogWindow(QWidget *parent = 0);
		~MealogWindow();

	signals:
		void newDataAvailable(void);

	private slots:
		void createNewRecording(void);
		void loadRecording(void);
		void closeRecordingWithCheck(void);
		//void startMeaview(void);
		//void acceptClients(void);
		//void respondToClient(void);
		void chooseSaveDir(void);
		void connectToDaqsrv(void);
		void disconnectFromDaqsrv(void);
		void handleDaqsrvConnection(bool made);
		void handleServerDisconnection(void);
		void handleServerError(void);
		void startRecording(void);
		void recvData(void);

	private:

		/* Initialisation functions */
		void initSettings(void);
		void initGui(void);
		void initMenuBar(void);
		void initPlotWindow(void);
		void initServer(void);
		void initSignals(void);

		/* Helpers */
		void setPlaybackButtonsEnabled(bool enabled);
		void sendDaqsrvInitMessage(void);
		//bool checkMeaviewRunning(void);
		bool confirmFileOverwrite(const QFile &path);
		int confirmCloseRecording(void);
		void closeRecording(void);
		void cleanupRecording(void);
		QString getFullFilename(void);
		bool deleteOldRecording(QFile &path);
		void setRecordingParameters(void);
		void setParameterSelectionsEnabled(bool enabled);
		void setNidaqInterfaceEnabled(bool enabled);
		mearec::RecordingStatusReply constructStatusReply(
				mearec::RecordingStatusRequest req);
		bool readMessage(QTcpSocket *socket, 
				mearec::RecordingStatusRequest &request);
		bool writeMessage(QTcpSocket *socket,
				mearec::RecordingStatusReply &reply);

		/* Server for responding to clients requesting 
		 * information about recording.
		 */
		QTcpServer *server = nullptr;

		/* Internals for getting data from NI-DAQ server and 
		 * writing it to disk.
		 */
		DaqClient *daqClient = nullptr;
		H5Recording *recording = nullptr;
		uint8_t recordingStatus = Mealog::INITIALIZED & Mealog::NOT_STARTED;
		uint64_t numSamplesAcquired = 0;

		/* The PlotWindow is the class containing the actual data plots */
		PlotWindow *plotWindow = nullptr;

		/* Various settings and constants used throughout */
		Settings settings;

		/* Main window's layout and status bar */
		QGridLayout *mainLayout;
		QStatusBar *statusBar;
		
		/* Menus */
		QMenuBar *menubar;
		QAction *aboutAction;
		QMenu *fileMenu;
		QAction *newRecordingAction;
		QAction *loadRecordingAction;
		QAction *closeRecordingAction;
		QMenu *windowsMenu;
		QAction *showPlotWindow;
		QAction *showControlsWindow;

		/* New/load and path stuff */
		QGroupBox *topGroup;
		QGridLayout *topLayout;
		QPushButton *newRecordingButton;
		QPushButton *loadRecordingButton;
		QLabel *fileLabel;
		QLineEdit *fileLine;
		QRegExpValidator *fileValidator;
		QLabel *pathLabel;
		QLineEdit *pathLine;
		QPushButton *choosePathButton;

		/* Playback controls */
		QGroupBox *playbackGroup;
		QGridLayout *playbackLayout;
		QLabel *timeLabel;
		QLineEdit *timeLine;
		QLineEdit *totalTimeLine;
		QIntValidator *totalTimeValidator;
		QPushButton *playButton;
		QPushButton *stopButton;
		QPushButton *skipBackButton;
		QPushButton *skipForwardButton;
		QPushButton *skipToBeginningButton;
		QPushButton *skipToEndButton;

		/* NIDAQ stuff */
		QGroupBox *nidaqGroup;
		QGridLayout *nidaqLayout;
		QPushButton *connectToNidaqButton;
		QLabel *nidaqHostLabel;
		QLineEdit *nidaqHost;
		QRegExpValidator *nidaqValidator;
		QLabel *nidaqStatusLabel;
		QLineEdit *nidaqStatus;

		/*
		QGroupBox *ctrlGroup;
		QGridLayout *ctrlLayout;
		QPushButton *initRecordingButton;
		QPushButton *startMeaviewButton;
		QPushButton *startButton;
		QPushButton *quitButton;
		*/

		/* Recording parameters */
		QGroupBox *recordingGroup;
		QGridLayout *recordingLayout;
		QLabel *adcRangeLabel;
		QComboBox *adcRangeBox;
		QComboBox *triggerBox;
		QLabel *triggerLabel;

		/* Display parameters */
		QGroupBox *displayGroup;
		QGridLayout *displayLayout;
		QLabel *viewLabel;
		QComboBox *viewBox;
		QLabel *scaleLabel;
		QComboBox *scaleBox;
		QLabel *colorLabel;
		QComboBox *colorBox;
		QLabel *automeanLabel;
		QCheckBox *automeanBox;
		QLabel *autoscaleLabel;
		QCheckBox *autoscaleBox;
		QLabel *jumpSizeLabel;
		QSpinBox *jumpSize;

		/* Eventually online analysis stuff too */
};

#endif
