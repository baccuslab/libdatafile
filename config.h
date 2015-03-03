/* config.h
 * Configuration parameters for the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

/* C++ includes */
#include <cstring>
#include <string>
#include <vector>
#include <map>

#if QT_VERSION < 0x050000
/* Qt includes */
#endif

using namespace std;

/* Parameters of the main window */
const int WINDOW_HEIGHT = 1000;
const int WINDOW_WIDTH = 1000;

/* Information about channels */
const int PHOTODIODE_CHANNEL = 0;
const int INTRACELLULAR_CHANNEL_VOLTAGE = 1;
const int INTRACELLULAR_CHANNEL_CURRENT = 2;
const int EXTRA_INTRACELLULAR_CHANNEL = 3;

/* Simple struct representing the row and column of a plot */
typedef struct {
	int row;
	int col;
} pos_t;

/* Default location to save new files */
const string DEFAULT_SAVE_FILENAME("default-data");
const string SAVE_FILE_EXTENSION(".bin");
#ifdef Q_OS_WIN
const string DEFAULT_SAVE_DIR("C:/Desktop/");
#else
const string DEFAULT_SAVE_DIR("~/Desktop");
#endif
const unsigned int DEFAULT_RECORD_LENGTH = 1000; 	// seconds
const unsigned int MAX_RECORD_LENGTH = 10000; 		// seconds

/* Information about AIB binary files */
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
const float DEFAULT_DISPLAY_SCALE = 0.5;
const float DISPLAY_SCALES[] {
	0.125, 0.25, 0.5, 1, 2, 4
};
const float DISPLAY_REFRESH_INTERVAL = 2000; // ms

/* Possible arrangements of the electrodes in the window */
const string DEFAULT_VIEW("Hexagonal");
const vector<string> VIEW_LABELS { 
	"Hexagonal", 
	"Low density", 
	"High density", 
	"Channel order" 
};

/* The arragements themselves */
const vector<pos_t> HEXAGONAL_VIEW {
	{.row = 0, .col = 0},
	{.row = 0, .col = 1},
	{.row = 0, .col = 2},
	{.row = 0, .col = 3},
	{.row = 0, .col = 4},
	{.row = 0, .col = 5},
	{.row = 0, .col = 6},
	{.row = 0, .col = 7},
	{.row = 1, .col = 0},
	{.row = 1, .col = 1},
	{.row = 1, .col = 2},
	{.row = 1, .col = 3},
	{.row = 1, .col = 4},
	{.row = 1, .col = 5},
	{.row = 1, .col = 6},
	{.row = 1, .col = 7},
	{.row = 2, .col = 0},
	{.row = 2, .col = 1},
	{.row = 2, .col = 2},
	{.row = 2, .col = 3},
	{.row = 2, .col = 4},
	{.row = 2, .col = 5},
	{.row = 2, .col = 6},
	{.row = 2, .col = 7},
	{.row = 3, .col = 0},
	{.row = 3, .col = 1},
	{.row = 3, .col = 2},
	{.row = 3, .col = 3},
	{.row = 3, .col = 4},
	{.row = 3, .col = 5},
	{.row = 3, .col = 6},
	{.row = 3, .col = 7},
	{.row = 4, .col = 0},
	{.row = 4, .col = 1},
	{.row = 4, .col = 2},
	{.row = 4, .col = 3},
	{.row = 4, .col = 4},
	{.row = 4, .col = 5},
	{.row = 4, .col = 6},
	{.row = 4, .col = 7},
	{.row = 5, .col = 0},
	{.row = 5, .col = 1},
	{.row = 5, .col = 2},
	{.row = 5, .col = 3},
	{.row = 5, .col = 4},
	{.row = 5, .col = 5},
	{.row = 5, .col = 6},
	{.row = 5, .col = 7},
	{.row = 6, .col = 0},
	{.row = 6, .col = 1},
	{.row = 6, .col = 2},
	{.row = 6, .col = 3},
	{.row = 6, .col = 4},
	{.row = 6, .col = 5},
	{.row = 6, .col = 6},
	{.row = 6, .col = 7},
	{.row = 7, .col = 0},
	{.row = 7, .col = 1},
	{.row = 7, .col = 2},
	{.row = 7, .col = 3},
	{.row = 7, .col = 4},
	{.row = 7, .col = 5},
	{.row = 7, .col = 6},
	{.row = 7, .col = 7},
};

const map<string, vector<pos_t> > CHANNEL_VIEWS {
	{"Hexagonal", HEXAGONAL_VIEW}
};


#endif

