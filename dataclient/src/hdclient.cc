/* hdclient.cc
 *
 * Implementation of client of the HiDens data server
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "include/hdclient.h"

hdclient::HidensClient::HidensClient(const QString& hostname,
		const quint16& port, QObject* parent) 
	: dataclient::DataClient(hostname, port, parent)
{
	hostname_ = hostname;
	port_ = port;
}

hdclient::HidensClient::~HidensClient()
{
	disconnect();
}

void hdclient::HidensClient::initExperiment()
{
}

void hdclient::HidensClient::startRecording()
{
}

void hdclient::HidensClient::recvData(size_t nsamples, void* buffer)
{
}

QByteArray hdclient::HidensClient::recvData(size_t nsamples)
{
	return QByteArray();
}

void hdclient::HidensClient::checkDataAvailable()
{
}

void hdclient::HidensClient::sendConfiguration(const QString& file)
{
}
