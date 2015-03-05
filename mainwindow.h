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
		void setScale(int);
		void togglePlayback();
		//void plotNextDataBlock();
		void openSettings();
		void setAutoscale(int);

	private slots:
		void toggleVisible();
		void plot();
	
	private:
		/* Methods */
		void initSettings();
		void initMenuBar();
		void initToolBar();
		void initInfoWidget();
		void initRecordingControlsWidget();
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
		//QVector<double> PLOT_X;

		/* GUI attributes */
		QDockWidget *infoWidget;
		QMenuBar *menubar;
		QMenu *fileMenu;
		QMenu *windowsMenu;
		QToolBar *toolbar;

		QPushButton *startButton;
		QPushButton *settingsButton;
		QGroupBox *timeGroup;
		QLabel *timeLabel;
		QHBoxLayout *timeLayout;
		QLineEdit *timeLine;
		QIntValidator *timeValidator;
		QLabel *fileLabel;
		QLineEdit *fileLine;
		QGridLayout *infoWidgetLayout;

		QGroupBox *scaleGroup;
		QHBoxLayout *scaleLayout;
		QLabel *scaleLabel;
		QComboBox *scaleBox;
		QLabel *autoscaleLabel;
		QCheckBox *autoscaleCheckBox;

		QStatusBar *statusBar;
		QLabel *statusLabel;

		QTimer *playbackTimer;
		bool isPlaying = false;

		/* Recording attributes */
		PlaybackRecording *recording;

		CtrlWindow *ctrlWindow;
};

#endif

