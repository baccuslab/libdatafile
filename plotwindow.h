/* plotwindow.h
 * ------------
 * Header file for the data plotting window for the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _PLOTWINDOW_H_
#define _PLOTWINDOW_H_

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
#include "settings.h"

class PlotWindow : public QWidget {
	Q_OBJECT

	friend class CtrlWindow;
	public:
		PlotWindow(QWidget *parent = 0);
		~PlotWindow();

	public slots:
		void plotNextDataBlock();

	private slots:
		void toggleVisible();
	
	private:
		void initPlotGroup();
		ChannelPlot *channelPlot;
		Settings settings;
		PlaybackRecording *recording;
		QGridLayout *layout;
};

#endif

