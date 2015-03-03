/* recording.h
 * -----------
 * Header file for the Recording class, which represents a single
 * recording or playback of an experiment's data.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _RECORDING_H_
#define _RECORDING_H_

/* C++ includes */

/* Qt includes */
#if QT_VERSION < 0x050000
#endif

/* meaview includes */
#include "config.h"
#include "files.h"

/* class: Recording
 * ----------------
 * The Recording class is the base class for all experimental recordings,
 * whether an actual live recording or a playback from a file on disk.
 * The `LiveRecording` and `PlaybackRecording` classes inherit from this,
 * and should be preferred.
 */
class Recording {
	public:
		Recording(QString &filename, bool live, unsigned int time = 0);
		~Recording();

		bool getIsLive();
		DataFile &getFile();
		QString &getFilename();
		unsigned int getTime();

	protected:
		/* Methods */
		void setTime(unsigned int);
		void setFile(DataFile &file);

		/* Attributes */
		bool isLive;
		DataFile *dataFile;
		unsigned long currentBlock;
};

/* class: LiveRecording
 * --------------------
 * The `LiveRecording` class represents an actual ongoing live recording,
 * which streams data from the NI-DAQ ADC, and writes out to disk.
 */
class LiveRecording : public Recording {
	public:
		LiveRecording(QString &filename, unsigned int time);
		~LiveRecording();

	private:
		/* stuff about NI-DAQ has to be here */
};

/* class: PlaybackRecording
 * ------------------------
 * The `PlaybackRecording` class represents a playback of a previously-recorded
 * experiment. No data is streamed from the NI-DAQ ADC, and no data is written
 * to disk. Data is simply streamed from the given disk file.
 */
class PlaybackRecording : public Recording {
	public:
		PlaybackRecording(QString &filename);
		~PlaybackRecording();
		QVector<int16_t> getData(int block, int channel);
		QVector<QVector<int16_t> > getDataBlock(int block);
		QVector<QVector<int16_t> > getNextDataBlock();
};


#endif

