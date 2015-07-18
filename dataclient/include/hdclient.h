/* hdclient.h
 *
 * Public API for C++ class for interacting with HiDens data
 * server system.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef MEAREC_HDCLIENT_H_
#define MEAREC_HDCLIENT_H_

#include <QString>
#include <QHostAddress>
#include <QTcpSocket>
#include <QList>

#include "dataclient.h"

namespace hdclient {

const QString HIDENS_ADDRESS = "11.0.0.1";
const QHostAddress HIDENS_SERVER_ADDR(HIDENS_ADDRESS);
const quint16 HIDENS_SERVER_PORT = 11112;

typedef struct {
	size_t xpos;
	size_t ypos;
	size_t x;
	size_t y;
	char label;
	int channel;
} Electrode;

class HidensClient : public dataclient::DataClient {
	Q_OBJECT

	public:
		HidensClient(const QString& addr = HIDENS_ADDRESS, 
				const quint16& port = HIDENS_SERVER_PORT,
				QObject* parent = 0);
		HidensClient(const QHostAddress& addr = HIDENS_SERVER_ADDR,
				const quint16& port = HIDENS_SERVER_PORT, 
				QObject* parent = 0);
		HidensClient(const HidensClient& other) = delete;
		~HidensClient();

		virtual void initExperiment();
		virtual void startRecording();
		virtual void recvData(size_t nsamples, void* buffer);
		virtual QByteArray recvData(size_t nsamples);

		QString fpgaIP();
		size_t plug();
		void setPlug(size_t plug = 0);

		void sendConfiguration(const QString& file);
		size_t nConnectedChannels();
		QList<int> connectedChannels();
		QList<size_t> xpos();
		QList<size_t> ypos();
		QList<size_t> x();
		QList<size_t> y();
		QList<QChar> labels();

		size_t version();
		size_t chipid();
		QString timestamp();

		bool saving();
		void startSave(const QString& path = ".", 
				const QString& description = "");
		void stopSave();
	
		bool frameInHeader();
		void setFrameInHeader(bool on);

		void streamData(const size_t ms);
		void liveData(const size_t ms);

	private slots:
		virtual void checkDataAvailable();

	private:
		size_t plug_;

		QList<Electrode> configuration_;
		size_t nConnectedChannels_ = 0;
		QList<size_t> xpos_ = {};

};
};

#endif

