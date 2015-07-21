/* main.cc
 *
 * Main entry point for the daqsrv application.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "serverwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setOrganizationName("baccuslab");
	app.setApplicationName("daqsrv");

	serverwindow::ServerWindow win;
	win.show();

	return app.exec();
}
