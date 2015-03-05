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

/* meaview includes */
#include "settings.h"
#include "plotwindow.h"
#include "windows.h"
#include "files.h"
#include "recording.h"

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
		void chooseSaveDir();
		void updateTime();
		void updateView(QString);
		void updateColor(QString);
		void updateScale(QString);
		void updateJump(int);
		void updateAutoscale(int);
		void setOnlineAnalysisTargetChannel();
		void toggleVisible();
		void togglePlayback();

		void loadRecording();
		void openNewRecording();


	private:

		/* Initialization methods */
		void initSettings();
		void initCtrlWindowUI();
		void initMenuBar();
		void initPlotWindow();
		void initStatusBar();
		void initPlaybackRecording();
		void initLiveRecording();
		void initSignalsAndSlots();

		/* General attributes */
		Settings settings;
		QFile stimulusFile;
		unsigned int targetChannel;
		QTimer *playbackTimer;
		bool isPlaying = false;

		/* Data interface attributes */
		PlaybackRecording *recording;

		/* Main window GUI attributes */
		PlotWindow *plotWindow;
		QGridLayout *mainLayout;
		QMenuBar *menubar;
		QMenu *fileMenu;
		QMenu *windowsMenu;
		QStatusBar *statusBar;
		QLabel *statusLabel;

		/* Information display group */
		QGroupBox *infoGroup;
		QGridLayout *infoLayout;
		QLabel *filenameLabel;
		QRegExpValidator *filenameValidator;
		QLabel *savedirLabel;
		QPushButton *chooseSavedirButton;
		QLabel *timeLabel;
		QLineEdit *filenameLine;
		QLineEdit *savedirLine;
		QLineEdit *timeLine;
		QIntValidator *timeLineValidator;

		/* Playback controls group */
		QGroupBox *playbackGroup;
		QGridLayout *playbackLayout;
		QPushButton *restartButton;
		QPushButton *rewindButton;
		QPushButton *startPauseButton;
		QPushButton *stopButton;
		QPushButton *forwardButton;
		QPushButton *endButton;
		QLabel *jumpLabel;
		QSpinBox *jumpSpinBox;

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

		/* Interface with Nidaq */
		QGroupBox *nidaqGroup;
		QGridLayout *nidaqLayout;
};

#endif

