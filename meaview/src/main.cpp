/* main.cpp
 * Definition of entry point for meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "ctrlwindow.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	app.setOrganizationName("baccuslab");
	app.setApplicationName("meaview");
	CtrlWindow window;
	window.show();
	app.exec();
	return 0;
}
