/* ctrlwindow.h
 * ------------
 * Header file for the CtrlWindow class, which is the main interface
 * used for controlling a recording.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _CTRLWINDOW_H_
#define _CTRLWINDOW_H_

/* C++ includes */

/* Qt includes */
#include <QMainWindow>
#include <QGridLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>
#include <QIntValidator>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QTcpSocket>

/* meaview includes */
#include "settings.h"
#include "plotwindow.h"
#include "windows.h"

#include "h5recording/include/h5recording.h"
#include "mealog/include/mealog.h"

/* Constants */
const int MEALOG_SERVER_TIMEOUT = 100; // ms

/* class: CtrlWindow
 * -----------------
 * The `CtrlWindow` class describes the main interface through which
 * users control a recording. It displays information about the recording,
 * provides controls to start, stop and navigate the recording, and control
 * the playback display
 */
class CtrlWindow : public QMainWindow {
	Q_OBJECT

	friend class PlotWindow;
	public:
		CtrlWindow(QWidget *parent = 0);
		~CtrlWindow();

	private slots:
		void updateFilename();
		void chooseFile();
		void updateTime();
		void updateView(QString);
		void updateColor(QString);
		void updateScale(QString);
		void updateJump(int);
		void updateAutoscale(int);
		void setOnlineAnalysisTargetChannel();
		void toggleVisible();
		void togglePlayback();
		void updateRefreshInterval(int);
		void updateAutoMean(int);
		void openChannelInspectWindow(int);
		void updateTimeLine();
		void checkRecordingFile(const QString &path);
		void plotNextDataBlock();
		void loadRecording();

	private:

		/* Initialization methods */
		void initSettings();
		void initMealogClient();
		void initCtrlWindowUI();
		void initMenuBar();
		void initPlotWindow();
		void initStatusBar();
		void initPlayback();
		void addFileWatcher();
		//void initLiveRecording();
		void initSignalsAndSlots();
		QString requestFilenameFromMealog();

		/* General attributes */
		Settings settings;
		QFile stimulusFile;
		unsigned int targetChannel;
		QTimer *playbackTimer;
		bool isPlaying = false;
		size_t lastSampleIndex; // last sample plotted

		/* Networking stuff for interfacing with mealog */
		QTcpSocket *mealogClient;
		bool mealogConnected;

		/* Data interface attributes */
		//Playback *playback;
		H5Recording *recording = nullptr;
		QFileSystemWatcher *fileWatcher = nullptr;

		/* Main window GUI attributes */
		PlotWindow *plotWindow;
		QGridLayout *mainLayout;
		QMenuBar *menubar;
		QMenu *fileMenu;
		QMenu *windowsMenu;
		QStatusBar *statusBar;
		QLabel *statusLabel;

		/* File display group */
		QGroupBox *fileGroup;
		QGridLayout *fileLayout;
		QLabel *filenameLabel;
		QRegExpValidator *filenameValidator;
		QPushButton *chooseFileButton;
		QLineEdit *filenameLine;

		/* Playback controls group */
		QGroupBox *playbackGroup;
		QGridLayout *playbackLayout;
		QLabel *timeLabel;
		QLabel *timeLine;
		QPushButton *restartButton;
		QPushButton *rewindButton;
		QPushButton *startPauseButton;
		QPushButton *stopButton;
		QPushButton *forwardButton;
		QPushButton *endButton;
		QLabel *jumpLabel;
		QSpinBox *jumpSpinBox;
		QLabel *refreshLabel;
		QSpinBox *refreshSpinBox;

		/* Display control group */
		QGroupBox *displayGroup;
		QGridLayout *displayLayout;
		QLabel *viewLabel;
		QComboBox *viewBox;
		QLabel *colorLabel;
		QComboBox *colorBox;
		QLabel *scaleLabel;
		QComboBox *scaleBox;
		QLabel *autoscaleLabel;
		QCheckBox *autoscaleBox;
		QLabel *autoMeanLabel;
		QCheckBox *autoMeanBox;

		/* Online analysis controls group */
		QGroupBox *onlineAnalysisGroup;
		QGridLayout *onlineAnalysisLayout;
		QRadioButton *noneAnalysisButton;
		QRadioButton *temporalAnalysisButton;
		QRadioButton *linesAnalysisButton;
		QRadioButton *spatiotemporalAnalysisButton;
		QPushButton *loadStimulusButton;
		QPushButton *showAnalysisWindowButton;
		QLabel *targetChannelLabel;
		QLineEdit *targetChannelLine;
		QIntValidator *targetChannelValidator;
		QLabel *lengthLabel;
		QSpinBox *lengthSpinBox;
};

#endif

