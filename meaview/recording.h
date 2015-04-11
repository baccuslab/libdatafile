/* recording.h
 * -----------
 * Header file for the Recording class, which represents a single experimental recording.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _RECORDING_H_
#define _RECORDING_H_

/* Qt includes */
#include <QVector>

/* meaview includes */
#include "files.h"
#include "settings.h"

/* class: Recording
 * ----------------
 * The Recording class represents a single live data recording session in an experiment.
 */
class Recording : public QObject {
	Q_OBJECT

	public:
		Recording(QString &filename, unsigned int time = 0);
		~Recording();

		DataFile &getFile();
		QString &getFilename();
		unsigned int getTime();
		void setBlock(unsigned int);
		unsigned int getBlock();
		unsigned int getRecordingLength();
	
	private:
		/* Methods */
		void setTime(unsigned int);
		void setFile(DataFile &file);

		/* Attributes */
		DataFile *dataFile;
		unsigned long currentBlock;
		unsigned int recordingLength;
};


#endif

