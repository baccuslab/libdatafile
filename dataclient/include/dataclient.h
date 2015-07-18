/* dataclient.h
 *
 * Header describing base class for interacting with data servers for both the
 * MCS and HiDens array systems.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef MEAREC_DATAFILE_H_
#define MEAREC_DATAFILE_H_

#include <QString>
#include <QTcpSocket>
#include <QHostAddress>

namespace dataclient {

const size_t DATA_MESSAGE_BLOCK_SIZE = 1000; // samples

class DataClient : public QObject {
	Q_OBJECT

	public:
		DataClient(const QString& hostname, const quint16 port, 
				QObject* parent = 0);
		virtual ~DataClient();

		bool connected();	// Return true if connected to data server
		void connect();		// Connect to data server
		void disconnect();	// Disconnect from data server
		QString hostname();	// IP address of data server
		quint16 port();		// Port of data server

		virtual void initExperiment() = 0; 	// Perform any pre-experiment initialization
		virtual void startRecording() = 0;	// Send appropriate start message to data server

		void setSampleRate(size_t rate);// Set data sample rate
		size_t sampleRate();			// Return data sample rate
		virtual void setLength(float);	// Set length of the experiment
		virtual float length();			// Return length of the experiment
		virtual size_t nsamples();		// Return number of samples in experiment
		virtual void setAdcRange(float);
		virtual float adcRange();		// Return range of appropriate ADC stages
		virtual float gain();
		virtual size_t blockSize();		// Return data chunk block size (samples)
		virtual size_t nchannels();		// Return number of channels in experiment
		virtual void setTrigger(const QString&); // Set trigger for experiment
		virtual QString trigger();		// Return trigger used to start experiment
		virtual QString date();			// Return timestamp for experiment

		virtual QByteArray recvData(size_t nsamples) = 0;
		virtual void recvData(size_t nsamples, void* buffer) = 0;
		template<class T>
		void recvData(size_t nsamples, T& mat);

	signals:
		void connectionMade(bool made);
		virtual void dataAvailable(size_t nsamples);
		void disconnected();
		void error(void);

	private slots:
		virtual void checkDataAvailable() = 0;
		void connectionSuccessful();
		void connectionUnsuccessful();
		void handleDisconnection();
		void handleSocketError();

	protected:
		QTcpSocket *socket = nullptr;
		QDataStream *stream = nullptr;
		QString hostname_;
		QHostAddress host_;
		quint16 port_;
		bool connected_;

		float length_;
		size_t nsamples_;
		float adcRange_;
		float gain_;
		size_t blockSize_ = DATA_MESSAGE_BLOCK_SIZE;
		size_t nchannels_;
		size_t sampleRate_;
		QString trigger_;
		QString date_;
};
};

#endif

