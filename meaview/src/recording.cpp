/* recording.cpp
 * -------------
 * Implementation of classes representing recorded data in the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "recording.h"

Recording::Recording(QString &filename, unsigned int time) : QObject() {
	dataFile = new DataFile(filename, time);
	//qDebug() << "File: " << filename;
	//qDebug() << dataFile->getNumChannels() << endl;
	currentBlock = 0;
	recordingLength = dataFile->getNumSamples() / SAMPLE_RATE;
}

Recording::~Recording() {
}

DataFile &Recording::getFile() {
	return *(this->dataFile);
}

unsigned int Recording::getBlock() {
	return this->currentBlock;
}

unsigned int Recording::getRecordingLength() {
	return this->recordingLength;
}

void Recording::setBlock(unsigned int block) {
	this->currentBlock = block;
}


