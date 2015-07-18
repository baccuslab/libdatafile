/* dataclient.cc
 *
 * Implementation of base for client classes interacting with data servers.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "dataclient.h"

dataclient::DataClient::DataClient(const QString& hostname, 
		const quint16 port, QObject* parent) : QObject(parent) 
{
	socket = new QTcpSocket(this);
	stream = new QDataStream(socket);
	stream->setFloatingPointPrecision(QDataStream::SinglePrecision);
	hostname_ = hostname;
	port_ = port;
	host_ = QHostAddress(hostname);
	connected_ = false;
}

dataclient::DataClient::~DataClient()
{
	if (socket->isValid()) {
		socket->disconnectFromHost();
		socket->close();
	}
	delete socket;
	delete stream;
}

void dataclient::DataClient::connect()
{
	QObject::connect(socket, &QAbstractSocket::connected,
			this, &dataclient::DataClient::connectionSuccessful);
	QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(connectionUnsucessful()));
	socket->connectToHost(host_, port_);
}

void dataclient::DataClient::disconnect()
{
	QObject::disconnect(socket, &QAbstractSocket::disconnected,
			this, &dataclient::DataClient::handleDisconnection);
	QObject::disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(handleSocketError()));
	QObject::disconnect(socket, &QAbstractSocket::readyRead,
			this, &dataclient::DataClient::checkDataAvailable);
	socket->disconnectFromHost();
}

void dataclient::DataClient::connectionSuccessful()
{
	QObject::disconnect(socket, &QAbstractSocket::connected,
			this, &dataclient::DataClient::connectionSuccessful);
	QObject::disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(connectionUnsucessful()));
	QObject::connect(socket, &QAbstractSocket::disconnected,
			this, &dataclient::DataClient::handleDisconnection);
	QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(handleSocketError()));
	QObject::connect(socket, &QAbstractSocket::readyRead,
			this, &dataclient::DataClient::checkDataAvailable);
	emit connectionMade(true);
	connected_ = true;
}

void dataclient::DataClient::connectionUnsuccessful()
{
	QObject::disconnect(socket, &QAbstractSocket::connected,
			this, &dataclient::DataClient::connectionSuccessful);
	QObject::disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(connectionUnsuccessful()));
	emit connectionMade(false);
}

void dataclient::DataClient::handleDisconnection()
{
	if (socket->error() == QAbstractSocket::UnknownSocketError)
		emit disconnected();
}

void dataclient::DataClient::handleSocketError()
{
	emit error();
}

bool dataclient::DataClient::connected() { return connected_; }
QString dataclient::DataClient::hostname() { return hostname_; }
quint16 dataclient::DataClient::port() { return port_; }

size_t dataclient::DataClient::sampleRate() { return sampleRate_; }
void dataclient::DataClient::setSampleRate(size_t rate)
{
	sampleRate_ = rate;
}

float dataclient::DataClient::length() { return length_; }
void dataclient::DataClient::setLength(float len)
{
	length_ = len;
	nsamples_ = length_ * sampleRate_;
}

size_t dataclient::DataClient::nsamples() { return nsamples_; }
size_t dataclient::DataClient::blockSize() { return blockSize_; }

QString dataclient::DataClient::trigger() { return trigger_; }
void dataclient::DataClient::setTrigger(const QString& trig)
{
	trigger_ = trig;
}

size_t dataclient::DataClient::nchannels() { return nchannels_; }
void dataclient::DataClient::setAdcRange(float range) { adcRange_ = range; }
float dataclient::DataClient::adcRange() { return adcRange_; }
QString dataclient::DataClient::date() { return date_; }

