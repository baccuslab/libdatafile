/* daqclient.cpp
 * Implementation of the DaqClient class, which interfaces with the daqsrv
 * data acquisition server to stream data from an MCS array.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <string>
#include "daqclient.h"
#include "daqsrv/messages.h"

DaqClient::DaqClient(QString hostname, quint16 port) {
	socket = new QTcpSocket(this);
	stream = new QDataStream(socket);
	stream->setFloatingPointPrecision(QDataStream::SinglePrecision);
	hostname_ = hostname;
	port_ = port;
	host_ = QHostAddress(hostname);
	isConnected_ = false;
	nchannels_ = NUM_CHANNELS;
}

DaqClient::~DaqClient() {
	if (socket->isValid()) {
		socket->disconnectFromHost();
		socket->close();
	}
}

float DaqClient::length() {
	return this->length_;
}

uint64_t DaqClient::nsamples() {
	return this->nsamples_;
}

float DaqClient::adcRange() {
	return this->adcRange_;
}

uint32_t DaqClient::nchannels() {
	return this->nchannels_;
}

uint32_t DaqClient::blockSize() {
	return this->blockSize_;
}

QString DaqClient::trigger() {
	return this->trigger_;
}

QString DaqClient::date() {
	return this->date_;
}

void DaqClient::setLength(float length) {
	this->length_ = length;
	this->nsamples_ = length * SAMPLE_RATE;
	sizeOfDataMessage_ = (
			blockSize_ * nchannels_ * sizeof(int16_t) + 
			2 * sizeof(uint32_t) + sizeof(int16_t)
		);
}

void DaqClient::setAdcRange(float adcRange) {
	this->adcRange_ = adcRange;
}

void DaqClient::setBlockSize(uint32_t blockSize) {
	this->blockSize_ = blockSize;
}

void DaqClient::setTrigger(QString trigger) {
	this->trigger_ = trigger;
}

bool DaqClient::connectToDaqsrv(void) {
	connect(socket, SIGNAL(connected()), this, SLOT(connectionSuccessful()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), 
			this, SLOT(connectionUnsuccessful(QAbstractSocket::SocketError)));
	socket->connectToHost(host_, port_);
	return true;
}

void DaqClient::handleDisconnection(void) {
	if (socket->error() == QAbstractSocket::UnknownSocketError)
		emit disconnected();
}

void DaqClient::handleSocketError(QAbstractSocket::SocketError e) {
	emit error();
}

void DaqClient::connectionSuccessful(void) {
	emit connectionMade(true);
	disconnect(socket, SIGNAL(connected()), this, SLOT(connectionSuccessful()));
	disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)), 
			this, SLOT(connectionUnsuccessful(QAbstractSocket::SocketError)));
	connect(socket, SIGNAL(disconnected()), 
			this, SLOT(handleDisconnection()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(handleSocketError(QAbstractSocket::SocketError)));
	connect(socket, SIGNAL(readyRead()), 
			this, SLOT(checkDataAvailable()));
	isConnected_ = true;
}

void DaqClient::connectionUnsuccessful(QAbstractSocket::SocketError error) {
	disconnect(socket, SIGNAL(connected()), this, SLOT(connectionSuccessful()));
	disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)), 
			this, SLOT(connectionUnsuccessful(QAbstractSocket::SocketError)));
	emit connectionMade(false);
}

void DaqClient::checkDataAvailable(void) {
	if (socket->bytesAvailable() >= sizeOfDataMessage_)
		emit dataAvailable();
}

void DaqClient::disconnectFromDaqsrv(void) {
	socket->disconnectFromHost();
}

void DaqClient::initExperiment(void) {

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

void DaqClient::requestExptParams(void) {
	(*stream) << EXPT_PARAMS_REQ;
}

void DaqClient::sendClose(void) {
	(*stream) << CLOSE;
}

void DaqClient::sendError(void) {
	(*stream) << ERROR_MSG;
}

void DaqClient::recvExptParams(void) {
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

QString DaqClient::recvError(void) {
	uint32_t errSize;
	(*stream) >> errSize;
	QByteArray data = socket->read(errSize);
	return QString::fromStdString(data.toStdString());
}

void DaqClient::startRecording(void) {
	uint32_t type = START_EXPT;
	uint32_t msg_size = 8;
	(*stream) << type << msg_size;
	socket->flush();
}

QByteArray DaqClient::recvData(void) {
	uint32_t type;
	uint32_t nchannels, nsamples;
	(*stream) >> type;
	(*stream) >> nchannels >> nsamples;
	QByteArray data = socket->read(nchannels * nsamples * sizeof(int16_t));
	return data;
}

void DaqClient::recvData(int16_t *buffer) {
	uint32_t type, msg_size, nsamples;
	uint16_t nchannels;
	(*stream) >> type >> msg_size;
	(*stream) >> nchannels >> nsamples;
	qint64 nread = socket->read((char *) buffer, nchannels * nsamples * sizeof(int16_t));
	qDebug() << "Read" << nread << "bytes from socket";
}

bool DaqClient::isConnected(void) {
	return isConnected_;
}

