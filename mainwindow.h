/* mainwindow.h
 * ------------
 * Header file for the main GUI window for the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

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
#include "config.h"
#include "channelplot.h"
#include "recording.h"
#include "files.h"
#include "windows.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

	public:
		MainWindow(QWidget *parent = 0);
		~MainWindow();

	public slots:
		void openNewRecording();
		void loadRecording();
		void setScale(int);
		void togglePlayback();
		void plotNextDataBlock();
		void openSettings();
	
	private:
		/* Methods */
		void initSettings();
		void initMenuBar();
		void initToolBar();
		void initPlotGroup();
		void initStatusBar();
		void initPlaybackRecording();
		void initLiveRecording();

		/* Settings */
		QSettings *settings;

		/* GUI attributes */
		QMenuBar *menubar;
		QMenu *fileMenu;
		QToolBar *toolbar;

		QPushButton *startButton;
		QPushButton *settingsButton;
		QGroupBox *timeGroup;
		QLabel *timeLabel;
		QHBoxLayout *timeLayout;
		QLineEdit *timeLine;
		QIntValidator *timeValidator;

		QGroupBox *scaleGroup;
		QHBoxLayout *scaleLayout;
		QLabel *scaleLabel;
		QComboBox *scaleBox;

		QStatusBar *statusBar;

		QWidget *channelPlotGroup;
		QGridLayout *channelLayout;
		vector<ChannelPlot *> channelPlots;

		QTimer *playbackTimer;
		bool isPlaying = false;

		/* Recording attributes */
		PlaybackRecording *recording;
};

#endif

