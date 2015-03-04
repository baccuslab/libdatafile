/* windows.h
 * Header file for various window classes in the meaview application
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _WINDOWS_H_
#define _WINDOWS_H_

/* meaview includes */
#include "config.h"
#include "channelplot.h"
#include "files.h"
#include "recording.h"

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

/* class: NewRecordingWindow
 * -------------------------
 * A subclass of QDialog that allows the user to create
 * a new recording, choosing a few basic parameters of the recording.
 */
class NewRecordingWindow : public QDialog {
	Q_OBJECT
	public:
		NewRecordingWindow(QWidget *parent = 0);
		~NewRecordingWindow();

		QString getFullFilename();
		uint getTime();
		void openBinFile(QString &filename);
		int validateChoices();

	private slots:
		void setView();
		void chooseDirectory();

	private:
		QString getSaveDir();
		QString getSaveFilename();
		QString getView();

		QSettings *settings;
		QGroupBox *viewGroup;
		QVBoxLayout *viewLayout;
		QComboBox *viewBox;
		//QPushButton *viewButton;
		//QActionGroup *viewActionGroup;
		//QMenu *viewMenu;
		QGroupBox *saveGroup;
		QGridLayout *saveLayout;
		QLineEdit *saveLine;
		QPushButton *browseButton;
		QGroupBox *fileGroup;
		QRegExpValidator *fileValidator;
		QLineEdit *fileLine;
		QVBoxLayout *fileLayout;
		QGroupBox *timeGroup;
		QVBoxLayout *timeLayout;
		QLineEdit *timeLine;
		QIntValidator *timeValidator;
		QGroupBox *buttonGroup;
		QPushButton *okButton;
		QPushButton *cancelButton;
		QHBoxLayout *buttonLayout;
		QGridLayout *layout;
		QFile *file;
};

/* class: SettingsWindow
 * ---------------------
 * The SettingsWindow class provides a graphical interface for controlling
 * the various settings for the meaview application. These default values 
 * for these settings are contained in the header file `config.h`, and stored
 * in the running application as a QSettings object.
 */
class SettingsWindow : public QDialog {
	Q_OBJECT

	public:
		SettingsWindow(QWidget *parent = 0);
		~SettingsWindow();

	public slots:
		void applySettings();

	private:
		/* The settings object */
		QSettings *settings;

		/* GUI components */
		QGridLayout *mainLayout;
		QGroupBox *displayGroup; // arrangement, scale, pens
		QGridLayout *displayLayout;
		QLabel *viewLabel;
		QComboBox *viewBox;
		QLabel *scaleLabel;
		QComboBox *scaleBox;
		QLabel *penColorLabel;
		QComboBox *penColorBox;

		QGroupBox *playbackGroup; // speed of playback, maybe point to new file?
		QGridLayout *playbackLayout;
		QLabel *refreshLabel;
		QLineEdit *refreshLine;
		QIntValidator *refreshValidator;

		QGroupBox *nidaqGroup; 	// settings related to NI-DAQ interface

		QGroupBox *saveGroup;	// format/location for data to be saved
		QLabel *savedirLabel;
		QLineEdit *savedirLine;
		QPushButton *savedirBrowseButton;
		QLabel *savefileLabel;
		QLineEdit *savefileLine;

		QGroupBox *okGroup;
		QHBoxLayout *okLayout;
		QPushButton *okButton;
		QPushButton *applyButton;
		QPushButton *cancelButton;
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

