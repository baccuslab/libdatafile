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

const QString DEFAULT_HIDENS_HOST = "11.0.0.1";
const QHostAddress HIDENS_SERVER_ADDR(DEFAULT_HIDENS_HOST);
const quint16 HIDENS_SERVER_PORT = 11112;

typedef struct {
	size_t xpos;
	size_t ypos;
	size_t x;
	size_t y;
	QChar label;
	int channel;
} Electrode;

class HidensClient : public dataclient::DataClient {
	Q_OBJECT

	public:
		HidensClient(const QString& addr = DEFAULT_HIDENS_HOST, 
				const quint16& port = HIDENS_SERVER_PORT,
				QObject* parent = 0);
		HidensClient(const HidensClient& other) = delete;
		~HidensClient();

		virtual void initExperiment() Q_DECL_OVERRIDE;
		virtual void startRecording() Q_DECL_OVERRIDE;
		virtual void recvData(size_t nsamples, void* buffer) Q_DECL_OVERRIDE;
		virtual QByteArray recvData(size_t nsamples) Q_DECL_OVERRIDE;

		QString fpgaIP();
		size_t plug();
		void setPlug(size_t plug = 0);

		void sendConfiguration(const QString& file);
		QList<Electrode> configuration();
		QList<Electrode> allElectrodes();
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
		virtual void checkDataAvailable() Q_DECL_OVERRIDE;

	private:
		void askServer(const QString& cmd);
		QByteArray recvServer();
		int recvInt();
		QString recvStr();
		float recvFloat();
		QList<int> recvNumList(const QRegExp& re);

		/* Functions appended by "get" indicate requesting data 
		 * directly from the HiDens server, rather than returning
		 * it from a cached value 
		 */
		void getConfiguration();
		void getConnectedChannels();
		void getDataChannels();
		void getAllElectrodePositions();
		float getSampleRate();
		int getHidensVersion();
		QString getTimestamp();

		bool saving_;
		size_t plug_;
		QList<Electrode> allElectrodePositions_ = {};
		QList<Electrode> configuration_ = {};
		size_t nConnectedChannels_ = 0;
		QList<int> connectedChannels_ = {};
		QList<int> dataChannels_ = {};
		int hidensVersion_;
		QString timestamp_;
		int bytesAvailable_;
};
};

#endif

