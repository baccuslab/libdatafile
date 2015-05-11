/* daqclient.h
 * Header file describing the DaqClient class, which interfaces with the
 * daqsrv data acquisition server to stream data from and MCS array durin
 * an experiment.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _DAQ_CLIENT_H_
#define _DAQ_CLIENT_H_

#include <QString>
#include <QTcpSocket>
#include <QHostAddress>

#include "daqsrv/messages.h"

const quint16 PORT = 12345;	// Port of NIDAQ server application
const quint32 BLOCK_SIZE = NIDAQ_BLOCK_SIZE;
const qint16 NUM_CHANNELS = NIDAQ_NUM_CHANNELS;
const quint32 SAMPLE_RATE = NIDAQ_SAMPLE_RATE;

class DaqClient : public QObject {
	Q_OBJECT

	public:
		DaqClient(QString hostname, quint16 port = PORT);
		~DaqClient();

		/* Parameters */
		float length();			// Length of experiment in seconds
		uint64_t nsamples();	// Length of experiment in samples
		float adcRange();		// Voltage range of NI-DAQ
		uint32_t blockSize();	// Size in samples of blocks recvd in data messages
		uint32_t nchannels();	// Get number of channels
		QString trigger();		// Mechanism for triggering experiment
		QString date();			// Timestamp for experiment

		void setLength(float length);
		void setAdcRange(float adcRange);
		void setBlockSize(uint32_t blockSize);
		void setTrigger(QString trigger);

		/* Data */
		QByteArray recvData(qint64);		// Receive a data message 
		void recvData(qint64, int16_t *);	// Read data into buffer

		/* Check if currently connected to NIDAQ server */
		bool isConnected();

		/* Implementation of messages */
		bool connectToDaqsrv();		// Connect to the server
		void disconnectFromDaqsrv();// Disconnect from the server
		void initExperiment();		// Initialize expt with current params
		void requestExptParams();	// Request daqsrv return experimental params
		void recvExptParams(void);	// Receive experimental parameters
		void startRecording(void); 	// Send request to start streaming data
		QString recvError(void);	// Receive an error string
		void sendClose();			// Tell daqsrv to close up shop
		void sendError();			// Throw an error to the daqsrv

	signals:
		void connectionMade(bool made);	// Emitted when connection to server is made
		void dataAvailable(qint64);		// Emitted when data is available, with number of samples
		void disconnected(void);		// Emitted when disconnects from server normally
		void error(void);				// Emitted when disconnects from server with error

	private slots:
		void checkDataAvailable(void);
		void connectionSuccessful(void);
		void connectionUnsuccessful(void);
		void handleDisconnection(void);
		void handleSocketError(void);

	private:
		QTcpSocket *socket;
		QDataStream *stream;
		QString hostname_;
		QHostAddress host_;
		quint16 port_;

		/* Parameters */
		float length_;
		uint64_t nsamples_;
		float adcRange_;
		float adcResolution_;
		uint32_t blockSize_ = BLOCK_SIZE;
		uint32_t nchannels_ = NUM_CHANNELS;
		QString trigger_;
		QString date_;
	
		bool isConnected_;
		uint32_t sizeOfDataMessage_;
};

#endif

