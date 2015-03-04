/* files.h
 * Header file for various file readers/writers used in the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _FILES_H_
#define _FILES_H_

/* C++ includes */
#include <cstring>

/* Qt includes */
#include <QFile>
#include <QDataStream>
#include <QDebug>
#include <QTime>
#include <QDate>
#include <QVector>

/* meaview includes */
#include "settings.h"

/* class: BinHeader
 * ----------------
 * The BinHeader class represents the header of a data file in the binary
 * AIB format.
 */
class BinHeader {
	public:
		BinHeader();
		BinHeader(unsigned int length);
		~BinHeader();

		friend class DataFile;
		friend QDataStream &operator<<(QDataStream &, BinHeader &);
		friend QDataStream &operator>>(QDataStream &, BinHeader &);

	private:
		void createDateAndTime();
		void computeNumSamples(unsigned int);
		uint32_t getNumSamples();
		void computeHeaderSize();

		uint32_t size;
		int16_t type;
		int16_t version;
		uint32_t numSamples;
		uint32_t numChannels;
		int16_t *channels;
		float sampleRate;
		uint32_t blockSize;
		float gain;
		float offset;
		uint32_t dateSize;
		char *dateStr;
		uint32_t timeSize;
		char *timeStr;
		uint32_t roomSize;
		char *roomStr;
};

/* class: DataFile
 * ---------------
 * The DataFile class provides an interface to data recorded during an
 * experiment. As of 01 Mar 2015, the only supported file format is the
 * standard "AIB" binary format traditionally used in the lab's recording
 * software. This class should be extensible, so that files may be written
 * in other formats, e.g., HDF5, but this is not yet implemented.
 */
class DataFile : public QFile {
	Q_OBJECT

	public:
		DataFile(QString &filename, QObject *parent = 0);
		DataFile(QString &filename, unsigned int length, QObject *parent = 0);
		~DataFile();
		QVector<int16_t> getData(int block, int channel);

		uint32_t getHeaderSize();
		uint32_t getNumSamples();
		uint32_t getNumChannels();
		uint32_t getBlockSize();
		uint32_t getNumBlocks();
		float getGain();
		float getOffset();

	private:
		/* Methods */
		void computeNumBlocks();

		/* Attributes */
		bool newFile;
		QString filename;
		QDataStream *dataStream;
		BinHeader *hdr;
		uint32_t numBlocks;
};

/* Stream insertion/extraction operators for DataFile 
 */
QDataStream& operator<<(QDataStream &, BinHeader &);
QDataStream& operator>>(QDataStream &, BinHeader &);

#endif

