/* main.cpp
 * Definition of entry point for meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "mainwindow.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	MainWindow window;
	window.show();
	app.exec();
	return 0;
}
