/* daqclient.h
 * Header file describing the McsClient class, which interfaces with the
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

#include "dataclient.h"
#include "daqsrv/nidaq.h"
#include "daqsrv/messages.h"

namespace mcsclient {

const quint16 PORT = 12345;	// Port of NIDAQ server application
const quint32 BLOCK_SIZE = NIDAQ_BLOCK_SIZE;
const qint16 NUM_CHANNELS = NIDAQ_NUM_CHANNELS;
const quint32 SAMPLE_RATE = NIDAQ_SAMPLE_RATE;
const QString DEFAULT_MCS_HOST("127.0.0.1");

class McsClient : public dataclient::DataClient {
	Q_OBJECT

	public:
		McsClient(const QString& hostname, quint16 port = PORT, 
				QObject* parent = 0);
		virtual ~McsClient();

		virtual void setLength(float) Q_DECL_OVERRIDE;

		virtual QByteArray recvData(size_t nsamples) Q_DECL_OVERRIDE;
		virtual void recvData(size_t nsamples, void* buffer) Q_DECL_OVERRIDE;

		virtual void initExperiment();	// Initialize expt with current params
		void requestExptParams();		// Request daqsrv return experimental params
		void recvExptParams(void);		// Receive experimental parameters
		void startRecording(void); 		// Send request to start streaming data
		QString recvError(void);		// Receive an error string
		void sendClose();				// Tell daqsrv to close up shop
		void sendError();				// Throw an error to the daqsrv

	private slots:
		virtual void checkDataAvailable(void) Q_DECL_OVERRIDE;

	private:
		uint32_t sizeOfDataMessage_;
		float adcResolution_;

}; 	// End class
};	// End namespace

#endif

