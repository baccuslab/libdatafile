/* mealog.h
 * Header file for mealog application.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _MEALOG_H_
#define _MEALOG_H_

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
#include <QProcess>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QDir>

//#include "daqclient/include/daqclient.h"
#include "h5recording/include/h5recording.h"
#include "proto/logserver.pb.h"

/* Settings */
const int MEALOG_WINDOW_WIDTH = 300;
const int MEALOG_WINDOW_HEIGHT = 200;
const QFile DEFAULT_SAVE_FILE("default-data.h5");
const QDir DEFAULT_SAVE_DIR(QDir::homePath());
const unsigned int DEFAULT_EXPERIMENT_LENGTH = 1000;
const unsigned int MAX_EXPERIMENT_LENGTH = 10000;
const QList<double> ADC_RANGES = { 1, 2, 5, 10 };
const double DEFAULT_ADC_RANGE = 5;
const QStringList TRIGGERS = {"Photodiode", "None"};
const quint16 IPC_PORT = 44444;

class MealogWindow : public QMainWindow {
	Q_OBJECT

	public:
		MealogWindow(QWidget *parent = 0);
		~MealogWindow();

	private slots:
		void startMeaview(void);
		void acceptClients(void);
		void respondToClient(void);
		void initRecording(void);
		void deInitRecording(void);

	private:
		void initGui(void);
		void initServer(void);
		void initSignals(void);
		bool checkMeaviewRunning(void);
		bool confirmFileOverwrite(const QFile &path);
		QFile *getFullFilename(void);
		bool removeOldRecording(QFile &path);
		void setRecordingParameters(void);
		void setParameterSelectionsEnabled(bool enabled);
		mearec::RecordingStatusReply constructStatusReply(
				mearec::RecordingStatusRequest req);
		bool readMessage(QTcpSocket *socket, 
				mearec::RecordingStatusRequest &request);
		bool writeMessage(QTcpSocket *socket,
				mearec::RecordingStatusReply &reply);

		/* Server for responding to clients requesting information about recording */
		QTcpServer *server;

		/* Process starting meaview if requested */
		QProcess *meaviewProcess = nullptr;

		// DaqClient client;
		H5Recording *recording;
		bool recordingInitialized = false;
		mearec::RecordingStatusReply_StatusType recordingStatus = \
				mearec::RecordingStatusReply_StatusType_STOPPED;

		/* GUI elements */
		QGridLayout *mainLayout;
		QStatusBar *statusBar;

		/* NIDAQ stuff */
		bool nidaqConnected = false;
		QGroupBox *nidaqGroup;
		QGridLayout *nidaqLayout;
		QPushButton *connectButton;
		QLabel *nidaqStatusLabel;
		QLabel *nidaqStatus;

		QGroupBox *ctrlGroup;
		QGridLayout *ctrlLayout;
		QPushButton *initRecordingButton;
		QPushButton *startMeaviewButton;
		QPushButton *startButton;
		QPushButton *quitButton;

		QGroupBox *paramGroup;
		QGridLayout *paramLayout;
		QComboBox *adcRangeBox;
		QLabel *adcRangeLabel;
		QComboBox *triggerBox;
		QLabel *triggerLabel;
		QLineEdit *timeLine;
		QLabel *timeLabel;
		QIntValidator *timeValidator;
		QLineEdit *fileLine;
		QLabel *fileLabel;
		QRegExpValidator *fileValidator;
		QLineEdit *savedirLine;
		QLabel *savedirLabel;
		QPushButton *chooseDirButton;
};

#endif
