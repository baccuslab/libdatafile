/* ctrlwindow.h
 * Header file for the CtrlWindow class, which is the main interface
 * used for controlling a recording.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _CTRLWINDOW_H_
#define _CTRLWINDOW_H_

/* C++ includes */

/* Qt includes */
#include <QGridLayout>
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

/* class: CtrlWindow
 * -----------------
 * The `CtrlWindow` class describes the main interface through which
 * users control a recording. It displays information about the recording,
 * provides controls to start, stop and navigate the recording, and control
 * the playback display
 */
class CtrlWindow : public QWidget {
	Q_OBJECT

	friend class MainWindow;
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

	private:

		/* Methods */
		void initUI();
		void initSignalsAndSlots();

		/* Attributes */
		bool autoscale = false;
		QFile stimulusFile;
		unsigned int targetChannel;
		Settings settings;

		/* GUI attributes */
		QGridLayout *mainLayout;

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

		QGroupBox *nidaqGroup;
		QGridLayout *nidaqLayout;
};

#endif

