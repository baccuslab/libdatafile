/* hdclient.cc
 *
 * Implementation of client of the HiDens data server
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "include/hdclient.h"
#include <QRegExp>
#include <QDir>
#include <QFileInfo>

namespace hdclient {
const size_t BUFFER_SIZE = 4096;
const QRegExp RECORDING_CHANNEL_RE("(1\n)|(Error\ninvalid channel id\n)");
const QRegExp CHANNEL_MAP_RE("(\n{2,})(?=\\d{1,4})");
const QRegExp STATUS_RE("1\n");
const QRegExp ELECTRODE_RE("[xyp]");
};

hdclient::HidensClient::HidensClient(const QString& hostname,
		const quint16& port, QObject* parent) 
	: dataclient::DataClient(hostname, port, parent)
{
	hostname_ = hostname;
	port_ = port;
	askServer("list_rec");	// read channels that can be used to record data
	dataChannels_ = recvNumList(hdclient::RECORDING_CHANNEL_RE);
	getAllElectrodePositions();
	sampleRate_ = getSampleRate();
	hidensVersion_ = getHidensVersion();
	timestamp_ = getTimestamp();
	bytesAvailable_ = getBytesAvailable();
}

hdclient::HidensClient::~HidensClient()
{
	if (saving_)
		stopSave();
	if (socket->isValid()) {
		socket->disconnectFromHost();
		socket->close();
	}
}

void hdclient::HidensClient::startSave(const QString& path,
			const QString& description)
{
	QDir saveDir(path);
	if (!saveDir.exists())
		throw std::invalid_argument(
				"Requested save directory does not exist");
	/*
	QFileInfo f(saveDir);
	if (!f.permssion(QFile::WriteUser))
		throw std::invalid_argument(
				"Insufficient permissions to requested save directory");
	*/
	askServer(QString("save_start %1 %2").arg(
				saveDir.path()).arg(description).toLocal8Bit());
}

void hdclient::HidensClient::streamData(size_t ms)
{

}

void hdclient::HidensClient::stopSave()
{
	if (saving_)
		askServer("save_stop");
}

void hdclient::HidensClient::initExperiment()
{
	/* Not sure what to do here */
}

void hdclient::HidensClient::startRecording()
{
	/* Start streaming data */
}

void hdclient::HidensClient::recvData(size_t nsamples, void* buffer)
{
	socket->read((char *) buffer, nsamples * nchannels_ * sizeof(uint8_t));
}

QByteArray hdclient::HidensClient::recvData(size_t nsamples)
{
	return socket->read(nsamples * nchannels_ * sizeof(uint8_t));
}

void hdclient::HidensClient::sendConfiguration(const QString& file)
{
}

QList<hdclient::Electrode> hdclient::HidensClient::configuration()
{
	if (configuration_.empty())
		getConfiguration();
	return configuration_;
}

void hdclient::HidensClient::checkDataAvailable()
{
	qint64 nbytes = socket->bytesAvailable();
	qint64 bytesPerBlock = blockSize_ * nchannels_ * sizeof(uint8_t);
	qint64 blocksAvailable = nbytes / bytesPerBlock;
	qint64 samplesAvailable = blocksAvailable * blockSize_;
	if (samplesAvailable > 0)
		emit dataAvailable(samplesAvailable);
}

QList<size_t> hdclient::HidensClient::xpos()
{
	QList<size_t> out;
	for (auto& each : configuration_)
		out << each.xpos;
	return out;
}

QList<size_t> hdclient::HidensClient::ypos()
{
	QList<size_t> out;
	for (auto& each : configuration_)
		out << each.ypos;
	return out;
}

QList<size_t> hdclient::HidensClient::x()
{
	QList<size_t> out;
	for (auto& each : configuration_)
		out << each.x;
	return out;
}

QList<size_t> hdclient::HidensClient::y()
{
	QList<size_t> out;
	for (auto& each : configuration_)
		out << each.y;
	return out;
}

QList<QChar> hdclient::HidensClient::labels()
{
	QList<QChar> out;
	for (auto& each : configuration_)
		out << each.label;
	return out;
}

QList<int> hdclient::HidensClient::connectedChannels()
{
	QList<int> out;
	for (auto& each : configuration_)
		out << each.channel;
	return out;
}

size_t hdclient::HidensClient::nConnectedChannels()
{
	size_t sum = 0;
	for (auto& each : configuration_)
		sum += (each.channel == -1) ? 0 : 1;
	return sum;
}

void hdclient::HidensClient::setPlug(size_t p)
{
	if (!connected())
		return;
	askServer("select " + QByteArray::number((qulonglong) p));
	if (recvInt() != 1)
		throw std::runtime_error("Could not set plug to " + std::to_string(p));
	plug_ = p;
}

size_t hdclient::HidensClient::plug()
{
	return plug_;
}

void hdclient::HidensClient::askServer(const QString& cmd)
{
	if (socket->write(cmd.toLocal8Bit()) != cmd.size())
		throw std::runtime_error("Could not send command '" + 
				cmd.toStdString() + "' to server");
}

QByteArray hdclient::HidensClient::recvServer()
{
	if (socket->bytesAvailable() > 0)
		return socket->read(hdclient::BUFFER_SIZE);
	return QByteArray();
}

int hdclient::HidensClient::recvInt()
{
	bool ok;
	auto ret = recvServer().toInt(&ok);
	if (!ok)
		throw std::runtime_error(
				"Could not convert expected integer return value from server");
	return ret;
}

QString hdclient::HidensClient::recvStr()
{
	return QString(recvServer());
}

float hdclient::HidensClient::recvFloat()
{
	bool ok;
	auto ret = recvServer().toFloat(&ok);
	if (!ok)
		throw std::runtime_error(
				"Could not converted expected floating point value from server");
	return ret;
}

QList<int> hdclient::HidensClient::recvNumList(const QRegExp& re)
{
	QString raw(socket->read(hdclient::BUFFER_SIZE));
	if (raw.endsWith(" \n"))
		raw.remove(raw.size() - 2, raw.size());
	raw.remove(re);
	QList<int> ret;
	QList<QString> s = raw.split(QRegExp("\\s+"));
	for (auto& each : s)
		ret << each.toInt();
	return ret;
}

QString hdclient::HidensClient::fpgaIP()
{
	if (!connected())
		return QString();
	askServer("fpga_ip");
	return QString(recvServer());
}

void hdclient::HidensClient::getConfiguration()
{
	if (connectedChannels_.empty())
		getConnectedChannels();
	for (auto& chan : connectedChannels_) {
		askServer("el " + QByteArray::number(chan));
		QByteArray ret = recvServer();
	}
}

void hdclient::HidensClient::getConnectedChannels()
{
	if (dataChannels_.empty())
		getDataChannels();
	QByteArrayList ch;
	for (auto& each : dataChannels_)
		ch << QByteArray::number(each);
	QByteArray cmd("ch "); 
	cmd += ch.join(",");
	askServer(cmd);

	QString ret(recvServer());
	QStringList vals = ret.split("\n", QString::KeepEmptyParts);
	for (auto& val : vals) {
		if (val.length())
			connectedChannels_ << val.toInt();
		else
			connectedChannels_ << -1;
	}
}

void hdclient::HidensClient::getDataChannels()
{
	askServer("list_rec");
	dataChannels_ = recvNumList(hdclient::RECORDING_CHANNEL_RE);
}

void hdclient::HidensClient::getAllElectrodePositions()
{
	askServer("el all");
	QString ret;
	while (true)
		ret += recvServer();
	QStringList els = ret.split(QRegExp("\n")); // Each "NUM NUM xNUMyNUMpNUM" quintuplet
	size_t channel = 0;
	for (auto& el : els) {
		QStringList trip = el.split(QRegExp("\\s+")); // {"x" "y" "xNUMyNUMpNUM"}
		hdclient::Electrode electrode;
		electrode.x = trip[0].toInt();
		electrode.y = trip[1].toInt();
		QStringList last = trip[2].split(hdclient::CHANNEL_MAP_RE);
		electrode.xpos = last[0].toInt();
		electrode.ypos = last[1].toInt();
		electrode.label = last[2].data()[0];
		electrode.channel = channel;
		channel += 1;
	}
}

float hdclient::HidensClient::getSampleRate()
{
	askServer("fr");
	return recvFloat();
}

int hdclient::HidensClient::getHidensVersion()
{
	askServer("ver");
	return recvInt();
}

QString hdclient::HidensClient::getTimestamp()
{
	askServer("ts");
	return recvStr();
}

int hdclient::HidensClient::getBytesAvailable()
{
	askServer("bytesavail");
	return recvInt();
}

