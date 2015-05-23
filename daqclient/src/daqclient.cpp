/* daqclient.cpp
 * Implementation of the DaqClient class, which interfaces with the daqsrv
 * data acquisition server to stream data from an MCS array.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <string>
#include "daqclient.h"
#include "daqsrv/messages.h"

DaqClient::DaqClient::DaqClient(QString hostname, quint16 port) {
	socket = new QTcpSocket(this);
	stream = new QDataStream(socket);
	stream->setFloatingPointPrecision(QDataStream::SinglePrecision);
	hostname_ = hostname;
	port_ = port;
	host_ = QHostAddress(hostname);
	isConnected_ = false;
}

DaqClient::DaqClient::~DaqClient() {
	if (socket->isValid()) {
		socket->disconnectFromHost();
		socket->close();
	}
}

float DaqClient::DaqClient::length() {
	return this->length_;
}

uint64_t DaqClient::DaqClient::nsamples() {
	return this->nsamples_;
}

float DaqClient::DaqClient::adcRange() {
	return this->adcRange_;
}

uint32_t DaqClient::DaqClient::nchannels() {
	return this->nchannels_;
}

uint32_t DaqClient::DaqClient::blockSize() {
	return this->blockSize_;
}

QString DaqClient::DaqClient::trigger() {
	return this->trigger_;
}

QString DaqClient::DaqClient::date() {
	return this->date_;
}

void DaqClient::DaqClient::setLength(float length) {
	this->length_ = length;
	this->nsamples_ = length * SAMPLE_RATE;
	sizeOfDataMessage_ = (
			blockSize_ * nchannels_ * sizeof(int16_t) + 
			2 * sizeof(uint32_t) + sizeof(int16_t)
		);
}

void DaqClient::DaqClient::setAdcRange(float adcRange) {
	this->adcRange_ = adcRange;
}

void DaqClient::DaqClient::setBlockSize(uint32_t blockSize) {
	this->blockSize_ = blockSize;
}

void DaqClient::DaqClient::setTrigger(QString trigger) {
	this->trigger_ = trigger;
}

bool DaqClient::DaqClient::connectToDaqsrv(void) {
	connect(socket, &QAbstractSocket::connected, 
			this, &DaqClient::connectionSuccessful);
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(connectionUnsuccessful()));
	socket->connectToHost(host_, port_);
	return true;
}

void DaqClient::DaqClient::handleDisconnection(void) {
	if (socket->error() == QAbstractSocket::UnknownSocketError)
		emit disconnected();
}

void DaqClient::DaqClient::handleSocketError(void) {
	emit error();
}

void DaqClient::DaqClient::connectionSuccessful(void) {
	emit connectionMade(true);
	disconnect(socket, &QAbstractSocket::connected, 
			this, &DaqClient::connectionSuccessful);
	disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)), 
			this, SLOT(connectionUnsuccessful()));
	connect(socket, &QAbstractSocket::disconnected,
			this, &DaqClient::handleDisconnection);
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(handleSocketError()));
	connect(socket, &QAbstractSocket::readyRead,
			this, &DaqClient::checkDataAvailable);
	isConnected_ = true;
}

void DaqClient::DaqClient::connectionUnsuccessful(void) {
	disconnect(socket, &QAbstractSocket::connected, 
			this, &DaqClient::connectionSuccessful);
	disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)), 
			this, SLOT(connectionUnsuccessful()));
	emit connectionMade(false);
}

void DaqClient::DaqClient::checkDataAvailable(void) {
	qint64 nbytes = socket->bytesAvailable();
	qint64 bytesPerBlock = blockSize_ * nchannels_ * sizeof(int16_t);
	qint64 blocksAvailable = nbytes / bytesPerBlock;
	qint64 samplesAvailable = blocksAvailable * blockSize_;
	if (samplesAvailable > 0)
		emit dataAvailable(samplesAvailable);
}

void DaqClient::DaqClient::disconnectFromDaqsrv(void) {
	socket->disconnectFromHost();
}

void DaqClient::DaqClient::initExperiment(void) {

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

void DaqClient::DaqClient::requestExptParams(void) {
	(*stream) << EXPT_PARAMS_REQ;
}

void DaqClient::DaqClient::sendClose(void) {
	(*stream) << CLOSE;
}

void DaqClient::DaqClient::sendError(void) {
	(*stream) << ERROR_MSG;
}

void DaqClient::DaqClient::recvExptParams(void) {
	uint32_t type, msgSize, nchannels, triggerSize, dateSize;
	(*stream) >> type >> msgSize >> nchannels >> nsamples_ >>
			this->length_ >> this->adcRange_ >> this->adcResolution_ >>
			this->blockSize_ >> triggerSize;
	QByteArray t = socket->read(triggerSize);
	this->trigger_ = QString::fromStdString(t.toStdString());
	(*stream) >> dateSize;
	QByteArray d = socket->read(dateSize);
	this->date_ = QString::fromStdString(d.toStdString());
}

QString DaqClient::DaqClient::recvError(void) {
	uint32_t errSize;
	(*stream) >> errSize;
	QByteArray data = socket->read(errSize);
	return QString::fromStdString(data.toStdString());
}

void DaqClient::DaqClient::startRecording(void) {
	uint32_t type = START_EXPT;
	uint32_t msg_size = 8;
	(*stream) << type << msg_size;
	socket->flush();
}

QByteArray DaqClient::DaqClient::recvData(qint64 nsamples) {
	qint64 nbytes = nsamples * nchannels_ * sizeof(int16_t);
	return socket->read(nbytes);
}

void DaqClient::DaqClient::recvData(qint64 nsamples, int16_t *buffer) {
	qint64 nbytes = nsamples * nchannels_ * sizeof(int16_t);
	socket->read((char *) buffer, nbytes);
}

bool DaqClient::DaqClient::isConnected(void) {
	return isConnected_;
}

