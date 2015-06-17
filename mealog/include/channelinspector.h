/* channelinspector.h
 * Header for ChannelInspector class, an extra window for seeing a
 * particular data channel in detail.
 *
 *  (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _CHANNEL_INSPECTOR_H_
#define _CHANNEL_INSPECTOR_H_

#include <QGridLayout>
#include <QCloseEvent>

#include "settings.h"
#include "qcustomplot.h"

class ChannelInspector : public QWidget {
	Q_OBJECT
	public:
		ChannelInspector(QCustomPlot *parentPlot, QCPGraph *sourceGraph, 
				int channel, QWidget *parent = 0);
		~ChannelInspector();
		int getChannel(void);
		void updateSourceGraph(QCPGraph *);

	signals:
		void aboutToClose(int channel);

	public slots:
		void replot(void);

	private:
		void closeEvent(QCloseEvent *event);
		Settings settings;
		QGridLayout *layout;
		QCustomPlot *plot;
		QCPAxisRect *rect;
		QCPGraph *graph;
		QCPGraph *sourceGraph;
		int channel;
};

#endif

