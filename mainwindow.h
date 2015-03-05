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
#include "channelplot.h"
#include "recording.h"
#include "files.h"
#include "windows.h"
#include "ctrlwindow.h"
#include "settings.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

	public:
		MainWindow(QWidget *parent = 0);
		~MainWindow();

	public slots:
		void openNewRecording();
		void loadRecording();
		void togglePlayback();
		void plotNextDataBlock();

	private slots:
		void toggleVisible();
	
	private:
		/* Methods */
		void initSettings();
		void initMenuBar();
		void initPlotGroup();
		void initStatusBar();
		void initPlaybackRecording();
		void initLiveRecording();
		void initCtrlWindow();
		
		//void transferDataToPlots(QVector<QVector<int16_t> >);

		bool autoscale;

		/* TMP */
		ChannelPlot *channelPlot;

		/* Settings */
		Settings settings;

		/* GUI attributes */
		QMenuBar *menubar;
		QMenu *fileMenu;
		QMenu *windowsMenu;
		QStatusBar *statusBar;
		QLabel *statusLabel;
		QTimer *playbackTimer;
		bool isPlaying = false;

		/* Recording attributes */
		PlaybackRecording *recording;

		CtrlWindow *ctrlWindow;
};

#endif

