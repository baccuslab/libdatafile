/* recording.cpp
 * -------------
 * Implementation of classes representing recorded data in the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "recording.h"

Recording::Recording(QString &filename, bool live, unsigned int time) {
	isLive = live;
	if (live)
		dataFile = new DataFile(filename, time);
	else
		dataFile = new DataFile(filename);
	//qDebug() << "File: " << filename;
	//qDebug() << dataFile->getNumChannels() << endl;
	currentBlock = 0;
}

Recording::~Recording() {
}

DataFile &Recording::getFile() {
	return *(this->dataFile);
}

void Recording::setBlock(unsigned int block) {
	this->currentBlock = block;
}

LiveRecording::LiveRecording(QString &filename, unsigned int time) :
	Recording(filename, true, time) {
}

LiveRecording::~LiveRecording() {
}

PlaybackRecording::PlaybackRecording(QString &filename) :
	Recording(filename, false, 0) {
}

PlaybackRecording::~PlaybackRecording() {
}

QVector<int16_t> PlaybackRecording::getData(int block, int channel) {
	return this->getFile().getData(block, channel);
}

QVector<QVector<int16_t> > PlaybackRecording::getDataBlock(int block) {
	QVector<int16_t> tmp(this->getFile().getBlockSize());
	QVector<QVector<int16_t> > data(this->getFile().getNumChannels());
	for (auto chan = 0; chan < this->getFile().getNumChannels(); chan++)
		data[chan] = this->getData(block, chan);
	this->currentBlock += 1;
	return data;
}

QVector<QVector<int16_t> > PlaybackRecording::getNextDataBlock() {
	return this->getDataBlock(this->currentBlock);
}
