/* serverwindow.cc
 *
 * Implementation of main GUI window for daqsrv application
 */

#include "include/serverwindow.h"
#include "include/nidaq.h"

serverwindow::ServerWindow::ServerWindow(QWidget* parent)
	: QMainWindow(parent)
{
	initGui();
	setGeometry(0, 0, 
			serverwindow::WINDOW_WIDTH, serverwindow::WINDOW_HEIGHT);
}

serverwindow::ServerWindow::~ServerWindow()
{
}

void serverwindow::ServerWindow::initGui()
{
	mainLayout = new QGridLayout(this);
	parameterGroup = new QGroupBox("Recording parameters", this);
	adcRangeLabel = new QLabel("ADC range:", parameterGroup);
	adcRangeBox = new QComboBox(parameterGroup);
	for (auto& each : nidaq::ADC_RANGE_LIST)
		adcRangeBox->addItem(QString::number(each));
	parameterLayout = new QGridLayout(this);
	parameterLayout->addWidget(adcRangeLabel, 0, 0);
	parameterLayout->addWidget(adcRangeBox, 1, 0);
	parameterGroup->setLayout(parameterLayout);

	serverGroup = new QGroupBox("Data server", this);
	clientLabel = new QLabel("Clients:", serverGroup);
	clientList = new QListWidget(serverGroup);
	serverLayout = new QGridLayout(this);
	serverLayout->addWidget(clientLabel, 0, 0);
	serverLayout->addWidget(clientList, 1, 0);
	serverGroup->setLayout(serverLayout);
	
	mainLayout->addWidget(parameterGroup, 0, 0);
	mainLayout->addWidget(serverGroup, 1, 0);
	
	statusBar = new QStatusBar(this);
	statusBar->showMessage("Ready");
	setStatusBar(statusBar);
	setCentralWidget(new QWidget(this));
	centralWidget()->setLayout(mainLayout);
}

