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

const int WINDOW_XPOS = PLOT_WINDOW_WIDTH + 10;
const int WINDOW_YPOS = 0;
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 200;
const QFile DEFAULT_SAVE_FILE("default-data.h5");
const QDir DEFAULT_SAVE_DIR(QDir::homePath());
const unsigned int DEFAULT_EXPERIMENT_LENGTH = 1000;
const unsigned int MAX_EXPERIMENT_LENGTH = 10000;
const unsigned int RECORDING_FINISH_WAIT_TIME = 1000; // ms
const QList<double> ADC_RANGES = { 1, 2, 5, 10 };
const double DEFAULT_ADC_RANGE = 5;
const QStringList TRIGGERS = {"Photodiode", "None"};
const QString DEFAULT_NIDAQ_HOST = "127.0.0.1";
const QString IPC_HOST = "127.0.0.1";
const quint16 IPC_PORT = 44444;

enum INIT_STATUS:uint16_t {
	UNINITIALIZED = 0,
	INITIALIZED = 1
};

enum RECORDING_STATUS:uint16_t {
	NOT_STARTED = 1 << 1,
	STARTED = 1 << 2,
	FINISHED = 1 << 3,
	ERROR = 1 << 4
};

enum DATA_SOURCE:uint16_t {
	PLAYBACK = 1 << 5,
	RECORDING = 1 << 6
};

enum PLAYBACK_STATUS:uint16_t {
	PLAYING = 1 << 7,
	PAUSED = 1 << 8
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
		void recordingFinished(void);

	private slots:
		void createNewRecording(void);
		void loadRecording(void);
		void closeRecordingWithCheck(void);
		void endRecording(void);
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
		void pauseRecording(void);
		void restartRecording(void);
		void recvData(void);
		void checkReadyForPlotting(void);
		void plotNextPlaybackDataBlock(void);

		/* Playback control slots */
		void jumpForward(void);
		void jumpBackward(void);
		void jumpToBeginning(void);
		void jumpToEnd(void);
		void updateRefreshInterval(int val);
		void updateJumpSize(int val);
		void updateChannelView(const QString &view);
		void updateAutoscale(int state);
		void updateAutomean(int state);
		void updateDisplayScale(const QString &text);
		
		/* Update functions for display params */
		//void updateExperimentLength(void);
		//void updateChannelView(void);
		//void updatePlotScale(void);
		//void updateSkipSize(void);

	private:

		/* Initialisation functions */
		void initGui(void);
		void initMenuBar(void);
		void initPlotWindow(void);
		void initServer(void);
		void initSignals(void);
		void initPlayback(void);

		/* Helpers */
		void plotDataBlock(uint64_t startSample, uint64_t endSample);
		void setPlaybackButtonsEnabled(bool enabled);
		void setPlaybackMovementButtonsEnabled(bool enabled);
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
		void updateTime(void);
		void waitForRecordingFinish(void);

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
		uint8_t recordingStatus = Mealog::UNINITIALIZED | Mealog::NOT_STARTED;
		uint64_t numSamplesAcquired = 0;
		uint64_t lastSamplePlotted = 0;

		/* Playback stuff */
		QTimer *playbackTimer;

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
		QPushButton *closeRecordingButton;
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
		QPushButton *startButton;
		QPushButton *stopButton;
		QPushButton *jumpBackButton;
		QPushButton *jumpForwardButton;
		QPushButton *jumpToBeginningButton;
		QPushButton *jumpToEndButton;

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
		QLabel *refreshLabel;
		QSpinBox *refreshBox;
		QLabel *viewLabel;
		QComboBox *viewBox;
		QLabel *scaleLabel;
		QComboBox *scaleBox;
		QLabel *jumpSizeLabel;
		QSpinBox *jumpSizeBox;
		QLabel *automeanLabel;
		QCheckBox *automeanBox;
		QLabel *autoscaleLabel;
		QCheckBox *autoscaleBox;

		/* Eventually online analysis stuff too */
};

#endif
