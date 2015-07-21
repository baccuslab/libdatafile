/* serverwindow.h
 *
 * Header file describing main GUI window for the daqsrv application.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef DAQSRV_SERVERWINDOW_H_
#define DAQSRV_SERVERWINDOW_H_

#include <QMainWindow>
#include <QStatusBar>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>

namespace serverwindow {
const size_t WINDOW_WIDTH = 400;
const size_t WINDOW_HEIGHT = 200;

class ServerWindow : public QMainWindow {
	Q_OBJECT

	public:
		ServerWindow(QWidget* parent = 0);
		~ServerWindow();

	private:
		void initGui();

		/* GUI components */
		QStatusBar *statusBar;
		QGridLayout *mainLayout;

	
		QGroupBox *parameterGroup;
		QLabel *adcRangeLabel;
		QComboBox *adcRangeBox;
		QLabel *timeLabel;
		QLineEdit *currentTimeLine;
		QLineEdit *timeLine;
		QLabel *triggerLabel;
		QComboBox *triggerBox;
		QGridLayout *parameterLayout;

		QGroupBox *serverGroup;
		QLabel *clientLabel;
		QListWidget *clientList;
		QGridLayout *serverLayout;
		
		//daqsrv::NidaqServer server;

};

};

#endif

