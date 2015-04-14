/* windows.h
 * Header file for various window classes in the meaview application
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _WINDOWS_H_
#define _WINDOWS_H_

/* meaview includes */
#include "settings.h"
#include "channelplot.h"
#include "files.h"
#include "recording.h"

/* Forward declarations */
class CtrlWindow;

/* class: NewRecordingWindow
 * -------------------------
 * A subclass of QDialog that allows the user to create
 * a new recording, choosing a few basic parameters of the recording.
 */
//class NewRecordingWindow : public QDialog {
	//Q_OBJECT
	//public:
		//NewRecordingWindow(QWidget *parent = 0);
		//~NewRecordingWindow();

		//QString getFullFilename();
		//uint getTime();
		//void openBinFile(QString &filename);
		//int validateChoices();

	//private slots:
		//void setView();
		//void chooseDirectory();

	//private:
		//QString getSaveDir();
		//QString getSaveFilename();
		//QString getView();

		//Settings settings;
		//QGroupBox *viewGroup;
		//QVBoxLayout *viewLayout;
		//QComboBox *viewBox;
		////QPushButton *viewButton;
		////QActionGroup *viewActionGroup;
		////QMenu *viewMenu;
		//QGroupBox *saveGroup;
		//QGridLayout *saveLayout;
		//QLineEdit *saveLine;
		//QPushButton *browseButton;
		//QGroupBox *fileGroup;
		//QRegExpValidator *fileValidator;
		//QLineEdit *fileLine;
		//QVBoxLayout *fileLayout;
		//QGroupBox *timeGroup;
		//QVBoxLayout *timeLayout;
		//QLineEdit *timeLine;
		//QIntValidator *timeValidator;
		//QGroupBox *buttonGroup;
		//QPushButton *okButton;
		//QPushButton *cancelButton;
		//QHBoxLayout *buttonLayout;
		//QGridLayout *layout;
		//QFile *file;
//};

/* class: ChanneInspectWindow 
 * --------------------------
 * Window for zooming in on a single channel.
 */
class ChannelInspectWindow : public QWidget{
	Q_OBJECT
	public:
		ChannelInspectWindow(ChannelPlot *p, int channel,
				QWidget *parent = 0);
		~ChannelInspectWindow();
	public slots:
		void replot();

	private:
		Settings settings;
		ChannelPlot *channelPlot;
		QCustomPlot *plot;
		QCPAxisRect *rect;
		QCPGraph *graph;
		QGridLayout *layout;
		int index;
		QVector<double> yData;
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

