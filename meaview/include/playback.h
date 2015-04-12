/* playback.h
 * ----------
 * Header file describing the Playback class, which allows the meaview application
 * to play back a previously-recorded data file.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _PLAYBACK_H_
#define _PLAYBACK_H_

/* Qt includes */
#include <QVector>

/* meaview includes */
#include "files.h"
#include "settings.h"

/* class: Playback
 * ---------------
 * The `Playback` class represents a playback of a previously-recorded
 * experiment. No data is streamed from the NI-DAQ ADC, and no data is written
 * to disk. Data is simply streamed from the given disk file.
 */
class Playback : public QObject {
	Q_OBJECT

	public:
		Playback(QString &filename);
		~Playback();
		//QVector<int16_t> getData(int block, int channel);
		//QVector<QVector<int16_t> > getDataBlock(int block);
		//QVector<QVector<int16_t> > getNextDataBlock();


		QVector<QVector<double> > getNextDataBlock();

		DataFile &getFile();
		QString &getFilename();
		void setBlock(unsigned int);
		unsigned int getBlock();
		unsigned int getRecordingLength();

	private:
		QVector<double> getData(int block, int channel);
		QVector<QVector<double> > getDataBlock(int block);

	//signals:
		//void endOfPlaybackFile();

	private:
		Settings settings;
		QString filename;
		DataFile *dataFile;
		QVector<QVector<double> > currentDataBlock;
		QVector<QVector<double> > dataBuffer;
		unsigned int currentPlaybackPtr;
		unsigned int currentBlock;
		unsigned int recordingLength;
};

#endif

