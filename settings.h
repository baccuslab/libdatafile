/* settings.h
 * This header file defines configuration parameters used throughout
 * the meaview application, and a custom Settings class that stores
 * and retrieves that data.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu.
 */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

/* C++ includes */
#include <cstring>
#include <numeric>

/* Qt includes */
#include <QSettings>
#include <QString>
#include <QColor>
#include <QList>
#include <QStringList>
#include <QPair>
#include <QMap>
#include <QPen>

using namespace std;

/* Parameters of the main window */
const int WINDOW_HEIGHT = 1000;
const int WINDOW_WIDTH = 1000;

/* Information about channels */
const int PHOTODIODE_CHANNEL = 0;
const int INTRACELLULAR_CHANNEL_VOLTAGE = 1;
const int INTRACELLULAR_CHANNEL_CURRENT = 2;
const int EXTRA_INTRACELLULAR_CHANNEL = 3;

/* Default location to save new files */
const QString DEFAULT_SAVE_FILENAME("default-data");
const QString SAVE_FILE_EXTENSION(".bin");
#ifdef Q_OS_WIN
const QString DEFAULT_SAVE_DIR("C:/Desktop/");
#else
const QString DEFAULT_SAVE_DIR("~/Desktop");
#endif
const unsigned int DEFAULT_EXPERIMENT_LENGTH = 1000; 	// seconds
const unsigned int MAX_EXPERIMENT_LENGTH = 10000; 		// seconds

/* Information about AIB binary files */
/* NOTE: These are not defined as Qt types, since they
 * must be written as unencoded binary so that files here
 * are compatible with traditional AIB files
 */
const float SAMPLE_RATE = 10000.0;
const float GAIN = 0.00015258789;
const float OFFSET = -5.0;
const uint32_t NUM_CHANNELS = 64;
const int16_t AIB_TYPE = 2;
const int16_t AIB_VERSION = 1;
const char AIB_ROOM[] = {'r', 'e', 'c', 'o', 'r', 'd', 'e', 'd', ' ',
		'i', 'n', ' ', 'd', '2', '3', '9', '\0'};
const uint32_t AIB_ROOM_SIZE = strlen(AIB_ROOM);
const uint32_t AIB_BLOCK_SIZE = 20000;
const char TIME_FORMAT[] = "h:mm:ss AP"; 		// QTime format
const char DATE_FORMAT[] = "ddd, MMM dd, yyyy";	// QDate format

/* Display properties for recording/playback */
const float NEG_DISPLAY_RANGE = (2 << 10);
const float POS_DISPLAY_RANGE = (2 << 12);
const float DEFAULT_DISPLAY_SCALE = 0.5;
const QList<float> DISPLAY_SCALES {
		0.125, 0.25, 0.5, 1, 2, 4
};
const unsigned int DISPLAY_REFRESH_INTERVAL = 2000; // ms
const unsigned int MIN_REFRESH_INTERVAL = 100;
const unsigned int MAX_REFRESH_INTERVAL = 10000;

/* Plot colors */
const QString DEFAULT_PLOT_COLOR("Black");
const QMap<QString, QPair<QColor, QPen> > PLOT_COLOR_MAP {
	{"Black", {QColor(Qt::black), QPen(QColor(Qt::black))}},
	{"Blue", {QColor(76, 114, 176), QPen(QColor(76, 114, 176))}}, 
	{"Green", {QColor(85, 168, 104), QPen(QColor(85, 168, 104))}}, 
	{"Red", {QColor(196, 78, 82), QPen(QColor(196, 78, 82))}}
};
const QStringList PLOT_COLOR_STRINGS(PLOT_COLOR_MAP.uniqueKeys());

/* Possible arrangements of the electrodes in the window */
const QString DEFAULT_VIEW("Hexagonal");
const QStringList CHANNEL_VIEW_STRINGS { 
	"Hexagonal", 
	"Low density", 
	"High density", 
	"Channel order" 
};

/* The arragements themselves */
const QList<QPair<int, int> > HEXAGONAL_VIEW {
	{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7},
	{1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7},
	{2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7},
	{3, 0}, {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 6}, {3, 7},
	{4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 4}, {4, 5}, {4, 6}, {4, 7},
	{5, 0}, {5, 1}, {5, 2}, {5, 3}, {5, 4}, {5, 5}, {5, 6}, {5, 7},
	{6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 6}, {6, 7},
	{7, 0}, {7, 1}, {7, 2}, {7, 3}, {7, 4}, {7, 5}, {7, 6}, {7, 7},
};

const QMap<QString, QList<QPair<int, int> > > CHANNEL_VIEW_MAP {
	{"Hexagonal", HEXAGONAL_VIEW}
};

/* class: Settings
 * ---------------
 * The `Settings` class is a subclass of QSettings that simplifies
 * the retrieval and storing of the settings used throughout the
 * meaview application.
 */
class Settings {
	public:
		Settings();
		~Settings();
		void setObjectName(QString);

		/* Getters */
		float getDisplayRange();
		float getDisplayScale();
		QList<float> getDisplayScales();
		unsigned int getRefreshInterval();
		QString getPlotColorString();
		QColor getPlotColor();
		QPen getPlotPen();
		QStringList getPlotColorStrings();
		QString getChannelViewString();
		QList<QPair<int, int> > getChannelView();
		QStringList getChannelViews();
		QString getSaveDir();
		QString getSaveFilename();
		unsigned int getExperimentLength();

		/* Setters */
		void setDisplayScale(float);
		void setRefreshInterval(unsigned int);
		void setPlotColor(QString);
		void setChannelView(QString);
		void setSaveDir(QString);
		void setSaveFilename(QString);
		void setExperimentLength(unsigned int);
		
		QString objectName();
	private:
		QSettings settings;
};


#endif

