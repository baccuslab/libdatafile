/* playback.cpp
 * ------------
 * Implementation of Playback class, representing a data file played back
 * from disk.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "playback.h"

//QVector<int16_t> Playback::getData(int block, int channel) {
	//return this->getFile().getData(block, channel);
//}

//QVector<QVector<int16_t> > Playback::getDataBlock(int block) {
	//QVector<int16_t> tmp(this->getFile().getBlockSize());
	//QVector<QVector<int16_t> > data(this->getFile().getNumChannels());
	//for (auto chan = 0; chan < this->getFile().getNumChannels(); chan++)
		//data[chan] = this->getData(block, chan);
	//return data;
//}

//QVector<QVector<int16_t> > Playback::getNextDataBlock() {
	//QVector<QVector<int16_t> > data = this->getDataBlock(this->currentBlock);
	//this->currentBlock += 1;
	//if (this->currentBlock == this->getFile().getNumBlocks())
		//emit endOfPlaybackFile();
	//return data;
//}

Playback::Playback(QString &fname) : QObject() {
	filename = fname;
	dataFile = new DataFile(filename);
	currentBlock = 0;
	int plotBlockSize = (settings.getRefreshInterval() / 1000) * AIB_BLOCK_SIZE;
	dataBuffer.resize(NUM_CHANNELS);
	currentDataBlock.resize(NUM_CHANNELS);
	for (auto i = 0; i < PLAYBACK_BUFFER_NUM_BLOCKS; i++ ) {
		for (auto j = 0; j < NUM_CHANNELS; j++)
			dataBuffer[j] += dataFile->getData(currentBlock + i, j);
	}
	for (auto i = 0; i < NUM_CHANNELS; i++) {
		currentDataBlock[i].resize(plotBlockSize);
		for (auto k = 0; k < plotBlockSize; k++)
			currentDataBlock[i][k] = dataBuffer[i][k];
	}
}

Playback::~Playback() { };

DataFile &Playback::getFile() {
	return *(this->dataFile);
}

unsigned int Playback::getBlock() {
	return this->currentBlock;
}

unsigned int Playback::getRecordingLength() {
	return this->recordingLength;
}

void Playback::setBlock(unsigned int block) {
	this->currentBlock = block;
}

QVector<double> Playback::getData(int block, int channel) {
	return this->getFile().getData(block, channel);
}

QVector<QVector<double> > Playback::getDataBlock(int block) {
	QVector<double> tmp(this->getFile().getBlockSize());
	QVector<QVector<double> > data(this->getFile().getNumChannels());
	for (auto chan = 0; chan < this->getFile().getNumChannels(); chan++)
		data[chan] = this->getData(block, chan);
	return data;
}

QVector<QVector<double> > Playback::getNextDataBlock() {
	QVector<QVector<double> > data = this->getDataBlock(this->currentBlock);
	this->currentBlock += 1;
	//if (this->currentBlock == this->getFile().getNumBlocks())
		//emit endOfPlaybackFile();
	return data;
}

//void Playback::endOfPlaybackFile() {
//}
