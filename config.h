/* config.h
 * Configuration parameters for the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu.
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

/* C++ includes */
#include <string>
#include <vector>
#include <map>

/* Qt includes */

using namespace std;

/* Parameters of the main window */
const int WINDOW_HEIGHT = 1000;
const int WINDOW_WIDTH = 1000;

/* Information about channels */
const int SAMPLE_RATE = 10000;
const int NUM_CHANNELS = 64;
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
const string DEFAULT_SAVE_DIR("/tmp");
#endif

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

