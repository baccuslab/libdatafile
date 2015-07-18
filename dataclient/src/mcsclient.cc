/* mcsclient.cpp
 * Implementation of the McsClient class, which interfaces with the daqsrv
 * data acquisition server to stream data from an MCS array.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <string>
#include "mcsclient.h"
#include "daqsrv/messages.h"

mcsclient::McsClient::McsClient(const QString& hostname, 
		const quint16 port, QObject* parent) 
	: dataclient::DataClient(hostname, port, parent)
{
	nchannels_ = mcsclient::NUM_CHANNELS;
	sampleRate_ = mcsclient::SAMPLE_RATE;
	blockSize_ = mcsclient::BLOCK_SIZE;
}

mcsclient::McsClient::~McsClient() 
{
	if (socket->isValid()) {
		socket->disconnectFromHost();
		socket->close();
	}
}

void mcsclient::McsClient::setLength(float length) 
{
	dataclient::DataClient::setLength(length);
	sizeOfDataMessage_ = (
			blockSize_ * nchannels_ * sizeof(int16_t) + 
			2 * sizeof(uint32_t) + sizeof(int16_t)
		);
}

void mcsclient::McsClient::checkDataAvailable(void) 
{
	qint64 nbytes = socket->bytesAvailable();
	qint64 bytesPerBlock = blockSize_ * nchannels_ * sizeof(int16_t);
	qint64 blocksAvailable = nbytes / bytesPerBlock;
	qint64 samplesAvailable = blocksAvailable * blockSize_;
	if (samplesAvailable > 0)
		emit dataAvailable(samplesAvailable);
}

void mcsclient::McsClient::initExperiment(void) 
{

	/* Compute sizes for message */
	uint32_t triggerLength = trigger_.length();
	uint32_t msgSize = (
			sizeof(msg_t) + 3 * sizeof(uint32_t) + 
			triggerLength + 2 * sizeof(float)
		);

	/* Write to stream */
	(*stream) << (uint32_t) INIT_EXPERIMENT << (uint32_t) msgSize 
			<< (uint32_t) triggerLength;
	stream->writeRawData(trigger_.toStdString().data(), triggerLength);
	(*stream) << (float) length_ << (float) adcRange_ << (uint32_t) blockSize_;
	socket->flush();
}

void mcsclient::McsClient::requestExptParams(void) 
{
	(*stream) << EXPT_PARAMS_REQ;
}

void mcsclient::McsClient::sendClose(void) 
{
	(*stream) << CLOSE;
}

void mcsclient::McsClient::sendError(void) 
{
	(*stream) << ERROR_MSG;
}

void mcsclient::McsClient::recvExptParams(void) 
{
	uint32_t type, msgSize, nchannels, 
			 triggerSize, dateSize, nsamples, blockSize;
	(*stream) >> type >> msgSize >> nchannels >> nsamples >>
			length_ >> adcRange_ >> adcResolution_ >>
			blockSize >> triggerSize;
	QByteArray t = socket->read(triggerSize);
	nsamples_ = nsamples;
	trigger_ = QString::fromStdString(t.toStdString());
	(*stream) >> dateSize;
	QByteArray d = socket->read(dateSize);
	date_ = QString::fromStdString(d.toStdString());
}

QString mcsclient::McsClient::recvError(void) 
{
	uint32_t errSize;
	(*stream) >> errSize;
	QByteArray data = socket->read(errSize);
	return QString::fromStdString(data.toStdString());
}

void mcsclient::McsClient::startRecording(void) 
{
	uint32_t type = START_EXPT;
	uint32_t msg_size = 8;
	(*stream) << type << msg_size;
	socket->flush();
}

QByteArray mcsclient::McsClient::recvData(size_t nsamples) 
{
	qint64 nbytes = nsamples * nchannels_ * sizeof(int16_t);
	return socket->read(nbytes);
}

void mcsclient::McsClient::recvData(size_t nsamples, void* buffer) 
{
	qint64 nbytes = nsamples * nchannels_ * sizeof(int16_t);
	socket->read((char *) buffer, nbytes);
}

