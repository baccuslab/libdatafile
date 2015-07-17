/* hdclient.cc
 *
 * Implementation of client of the HiDens data server
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "include/hdclient.h"

hdclient::HidensClient::HidensClient(const QHostAddress& addr,
		const quint16& port, QObject* parent) : QObject(parent) 
{
	addr_ = addr;
	port_ = port;
}

hdclient::HidensClient::HidensClient(const QString& addr,
		const quint16& port, QObject* parent) : QObject(parent)
{
	addr_ = QHostAddress(addr);
	port_ = port;
}

hdclient::HidensClient::~HidensClient()
{
	disconnect();
}

void hdclient::HidensClient::connect()
{
	QObject::connect(&sock_, &QAbstractSocket::connected,
			this, &hdclient::HidensClient::connectionSuccessful);
	QObject::connect(&sock_, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(connectionUnsuccessful()));
	sock_.connectToHost(addr_, port_);
}

void hdclient::HidensClient::disconnect()
{
	sock_.disconnectFromHost();
}

void hdclient::HidensClient::connectionSuccessful()
{
	connected_ = true;
	QObject::disconnect(&sock_, SIGNAL(error(QAbstractSocket::SocketError)),
			this, SLOT(connectionUnsuccessful()));
}

void hdclient::HidensClient::connectionUnsuccessful()
{
}

