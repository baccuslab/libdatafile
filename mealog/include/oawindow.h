/* oawindow.h
 * ----------
 *
 * Header for online analysis window classes, main user interface to
 * various online analyses.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _OAWINDOW_H_
#define _OAWINDOW_H_

#include <QWidget>
#include <QDir>
#include <QList>
#include <QStringList>
#include <QPluginLoader>
#include <QComboBox>
#include <QSpinBox>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

#include <armadillo>
#include "qcustomplot.h"

#include "stimulus.h"
#include "oainterface.h"

const QString PLUGIN_PATH = "../online-analysis/oaplugin/lib/";
const unsigned int OAWINDOW_WIDTH = 600;
const unsigned int OAWINDOW_HEIGHT = 400;

class OAWindow : public QWidget
{
	Q_OBJECT
	public:
		OAWindow(QWidget *parent = 0);
		~OAWindow(void);

		void loadPlugins(void);
		int oaChannel(void) { return channel; };

	public slots:
		void toggleVisible(void);
		void runAnalysis(uint64_t start, double rate, arma::vec data);
		void chooseStimulusFile(void);
		void setAnalysis(int);

	signals:
		void setRunning(bool, int);
		
	private:
		void initGui(void);
		void initEmpty(void);
		void initPlot(void);
		void addPlugin(QObject *plugin);
		void toggleAnalysis(void);
		void setupPlot(void);
		bool checkDimensionMatch(void); // return true if user is fine with it

		QDir pluginDir;
		QStringList pluginNames;
		QList<OAInterface *> analyses;
		OAInterface *analysis;

		QGridLayout *layout;
		QLabel *instructions;
		QLabel *analysisDescription;
		QComboBox *analysisBox;
		QLabel *stimLabel;
		QLineEdit *stimLine;
		QPushButton *chooseStimButton;
		QLabel *channelLabel;
		QSpinBox *channelBox;
		QPushButton *runButton;

		QCustomPlot *plot;

		QString stimFile = "";
		Stimulus *stimulus = nullptr;
		bool running = false;
		int channel = 0;
};

#endif

