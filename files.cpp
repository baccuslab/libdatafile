/* files.cpp
 * Implementation of various file readers/writers for meaview application.
 * (C) Benjamin Naecker bnaecker@stanford.edu
 */

/* C++ includes */
#include <iostream>
#include <cstdlib>
/* Qt includes */

/* meaview includes */
#include "files.h"

using namespace std;

/* This constructor is used when the file already exists */
DataFile::DataFile(QString &name, QObject *parent) : QFile(name, parent) {
	this->newFile = false;
	this->filename = name;

	/* Open the file */
	this->open(QIODevice::ReadWrite);
	this->dataStream = new QDataStream(this);
	this->dataStream->setByteOrder(QDataStream::BigEndian);
	this->dataStream->setFloatingPointPrecision(QDataStream::SinglePrecision);

	/* This file already exists, read the header */
	this->hdr = new BinHeader();
	*(this->dataStream) >> *(this->hdr);
}

/* This constructor is used when the recording is new, i.e.,
 * the underlying file does not already exist.
 */
DataFile::DataFile(QString &name, unsigned int length, QObject *parent) : 
		QFile(name, parent) {
	this->newFile = true;
	this->filename = name;

	/* Open the file and data stream */
	this->open(QIODevice::ReadWrite);
	this->dataStream = new QDataStream(this);
	this->dataStream->setByteOrder(QDataStream::BigEndian);
	this->dataStream->setFloatingPointPrecision(QDataStream::SinglePrecision);

	/* Construct a header and write it to the file */
	this->hdr = new BinHeader(length);
	*(this->dataStream) << *(this->hdr);
	this->flush();

	/* Resize the file for to the expected size */
	
}

DataFile::~DataFile() {
	delete this->dataStream;
	this->close();
}

QVector<int16_t> DataFile::getData(int block, int channel) {
	if (!(this->seek(this->hdr->size + 
			block * this->hdr->blockSize * this->hdr->numChannels * sizeof(int16_t) + 
			channel * this->hdr->blockSize * sizeof(int16_t))))
		throw;
	qDebug() << "Read start pos: " << this->pos();
	QVector<int16_t> data(this->hdr->blockSize);
	for (auto i = 0; i < this->hdr->blockSize; i++)
		*(this->dataStream) >> data[i];
	qDebug() << "Read end pos: " << this->pos();
	return data;
}

uint32_t DataFile::getHeaderSize() {
	return this->hdr->size;
}

uint32_t DataFile::getNumSamples() {
	return this->hdr->numSamples;
}

uint32_t DataFile::getNumChannels() {
	return this->hdr->numChannels;
}

uint32_t DataFile::getBlockSize() {
	return this->hdr->blockSize;
}

float DataFile::getGain() {
	return this->hdr->gain;
}

float DataFile::getOffset() {
	return this->hdr->gain;
}

QDataStream &operator<<(QDataStream &s, BinHeader &hdr) {
	s << hdr.size << hdr.type << hdr.version 
			<< hdr.numSamples << hdr.numChannels;
	for (auto i = 0; i < hdr.numChannels; i++)
		s << hdr.channels[i];
	s << hdr.sampleRate << hdr.blockSize << hdr.gain << hdr.offset;
	s << hdr.dateSize;
	s.writeRawData(hdr.dateStr, hdr.dateSize); 
	s << hdr.timeSize;
	s.writeRawData(hdr.timeStr, hdr.timeSize);
	s << hdr.roomSize;
	s.writeRawData(hdr.roomStr, hdr.roomSize);
	return s;
}

QDataStream &operator>>(QDataStream &s, BinHeader &hdr) {
	//s >> hdr.size >> hdr.type >> hdr.version 
			//>> hdr.numSamples >> hdr.numChannels;
	s >> hdr.size;
	s >> hdr.type;
	s >> hdr.version;
	s >> hdr.numSamples;
	s >> hdr.numChannels;
	qDebug() << "type: " << hdr.type;
	qDebug() << "vesion: " << hdr.version;
	qDebug() << "numSamples: " << hdr.numSamples;
	qDebug() << "numChannels: " << hdr.numChannels;
	if ((hdr.channels = (int16_t *) calloc(hdr.numChannels, sizeof(int16_t))) == nullptr)
		throw;
	for (auto i = 0; i < hdr.numChannels; i++)
		s >> hdr.channels[i];
	s >> hdr.sampleRate >> hdr.blockSize >> hdr.gain >> hdr.offset;
	qDebug() << "sampleRate: " << hdr.sampleRate;
	qDebug() << "blockSize: " << hdr.blockSize;
	qDebug() << "gain: " << hdr.gain;
	qDebug() << "offset: " << hdr.offset;
	s >> hdr.dateSize;
	if ((hdr.dateStr = (char *) calloc(1, hdr.dateSize + 1)) == nullptr)
		throw;
	s.readRawData(hdr.dateStr, hdr.dateSize); 
	s >> hdr.timeSize;
	if ((hdr.timeStr = (char *) calloc(1, hdr.timeSize + 1)) == nullptr)
		throw;
	s.readRawData(hdr.dateStr, hdr.dateSize); 
	s >> hdr.roomSize;
	if ((hdr.roomStr = (char *) calloc(1, hdr.roomSize + 1)) == nullptr)
		throw;
	s.readRawData(hdr.roomStr, hdr.roomSize);
	return s;
}

/* This constructor is used when the file is new */
BinHeader::BinHeader(unsigned int length) {
	this->version = AIB_VERSION;
	this->type = AIB_TYPE;
	this->numChannels = NUM_CHANNELS;
	if ((this->channels = (int16_t *) calloc(this->numChannels, sizeof(int16_t))) == nullptr)
		throw;
	for (auto i = 0; i < this->numChannels; i++)
		this->channels[i] = i;
	this->gain = GAIN;
	this->offset = OFFSET;
	this->sampleRate = SAMPLE_RATE;
	this->roomSize = AIB_ROOM_SIZE;
	if ((this->roomStr = (char *) calloc(1, this->roomSize + 1)) == nullptr)
		throw;
	memcpy(this->roomStr, AIB_ROOM, this->roomSize);
	this->createDateAndTime();
	this->computeNumSamples(length);
	this->computeHeaderSize();
}

/* This construct is used when the file already exists */
BinHeader::BinHeader() {
}

BinHeader::~BinHeader() {
}

void BinHeader::createDateAndTime() {
	QTime t = QTime::currentTime();
	this->timeSize = t.toString(TIME_FORMAT).size();
	if ((this->timeStr = (char *) calloc(1, this->timeSize + 1)) == nullptr)
		throw;
	if ((strlcpy(this->timeStr, t.toString(TIME_FORMAT).toStdString().c_str(), 
				this->timeSize + 1)) != this->timeSize)
		throw;
	QDate d = QDate::currentDate();
	this->dateSize = d.toString(DATE_FORMAT).size();
	if ((this->dateStr = (char *) calloc(1, this->dateSize + 1)) == nullptr)
		throw;
	if ((strlcpy(this->dateStr, d.toString(DATE_FORMAT).toStdString().c_str(), 
				this->dateSize + 1)) != this->dateSize)
		throw;
	//qDebug() << this->dateStr;
	//qDebug() << this->timeStr;
}

void BinHeader::computeNumSamples(unsigned int length) {
	this->numSamples = (uint32_t) (length * this->sampleRate);
}

uint32_t BinHeader::getNumSamples() {
	return this->numSamples;
}

void BinHeader::computeHeaderSize() {
	this->size = (
			7 * sizeof(uint32_t) + (2 + this->numChannels) * sizeof(int16_t) +
			3 * sizeof(float) + this->dateSize + this->timeSize + this->roomSize
			);
}

