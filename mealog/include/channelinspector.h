/* channelinspector.h
 * Header for ChannelInspector class, an extra window for seeing a
 * particular data channel in detail.
 *
 *  (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _CHANNEL_INSPECTOR_H_
#define _CHANNEL_INSPECTOR_H_

#include <QGridLayout>

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

	public slots:
		void replot(void);

	private:
		Settings settings;
		QGridLayout *layout;
		QCustomPlot *plot;
		QCPAxisRect *rect;
		QCPGraph *graph;
		QCPGraph *sourceGraph;
		int channel;
};

#endif

