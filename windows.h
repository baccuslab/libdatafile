/* windows.h
 * Header file for various window classes in the meaview application
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _WINDOWS_H_
#define _WINDOWS_H_

/* C++ includes */
#include <memory>		// For smart pointers

/* Qt includes */
//#include <QMainWindow>
//#include <QGridLayout>
//#include <QMenuBar>
//#include <QMenu>
//#include <QStatusBar>
//#include <QDialog>
//#include <QSettings>

/* meaview includes */
#include "config.h"
#include "channelplot.h"

/* class: MainWindow
 * -----------------
 * The MainWindow class is the main application window. It contains
 * the plots of data from each channel, various menus for controlling
 * recordings, and options to display various online analyses of 
 * selected channels.
 */
class MainWindow : public QMainWindow {
	Q_OBJECT

	public:
		MainWindow(QWidget *parent = 0);
		~MainWindow();

	public slots:
		void openNewRecording();
		void loadRecording();
	
	private:
		/* Methods */
		void initMenuBar();
		void initPlotGroup();
		void initStatusBar();

		/* Atributes */
		QMenuBar *menubar;
		QMenu *fileMenu;
		QStatusBar *statusBar;
		unique_ptr<QWidget> channelPlotGroup;
		unique_ptr<QGridLayout> channelLayout;
		vector<ChannelPlot *> channelPlots;
};

/* class: NewRecordingWindow
 * -------------------------
 * A simple subclass of QDialog that allows the user to create
 * a new recording, choosing a few basic parameters of the recording.
 */
class NewRecordingWindow : public QDialog {
	Q_OBJECT
	public:
		NewRecordingWindow(QWidget *parent = 0);
		~NewRecordingWindow();

		QString getFullFilename();
		void openBinFile(QString &filename);
		int validateChoices();

	private slots:
		void setView();

	private:
		QString getSaveDir();
		QString getSaveFilename();
		QString getView();

		QSettings *settings;
		QGroupBox *viewGroup;
		QVBoxLayout *viewLayout;
		QPushButton *viewButton;
		QActionGroup *viewActionGroup;
		QMenu *viewMenu;
		QGroupBox *saveGroup;
		QGridLayout *saveLayout;
		QLineEdit *saveLine;
		QPushButton *browseButton;
		QGroupBox *fileGroup;
		QRegExpValidator *fileValidator;
		QLineEdit *fileLine;
		QVBoxLayout *fileLayout;
		QGroupBox *buttonGroup;
		QPushButton *okButton;
		QPushButton *cancelButton;
		QHBoxLayout *buttonLayout;
		QGridLayout *layout;
		QFile *file;
};




/* class: OnlineAnalysisWindow
 * ---------------------------
 * The OnlineAnalysisWindow class is a base class for displaying the
 * state of a running online analysis. The useful inherited classes
 * are `SpatiotemporalOnlineAnalysisWindow`, `LinesOnlineAnalysisWindow`,
 * and `TemporalOnlineAnalysisWindow`.
 */
class OnlineAnalysisWindow : public QWidget {
	Q_OBJECT
};

/* class: TemporalOnlineAnalysisWindow
 * -----------------------------------
 * The TemporalOnlineAnalysisWindow class displays a running online analysis
 * of a purely temporal white noise stimulus.
 */
class TemporalOnlineAnalysisWindow : public OnlineAnalysisWindow {
	Q_OBJECT
};


/* class: LinesOnlineAnalysisWindow
 * --------------------------------
 * The LinesOnlineAnalysisWindow class displays a running online analysis
 * of a spatiotemporal white noise lines stimulus.
 */
class LinesOnlineAnalysisWindow : public OnlineAnalysisWindow {
	Q_OBJECT
};

/* class: SpatiotemporalOnlineAnalysisWindow
 * -----------------------------------------
 * The SpatiotemporalOnlineAnalysisWindow class displays a running online analysis
 * of a spatiotemporal white noise checkerboard stimulus.
 */
class SpatioTemporalOnlineAnalysisWindow : public OnlineAnalysisWindow {
	Q_OBJECT
};

#endif

