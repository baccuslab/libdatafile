/* main.cpp
 * Entry point for mealog application 
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <QApplication>
#include "mealogwindow.h"

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	app.setOrganizationName("baccuslab");
	app.setApplicationName("mealog");
	MealogWindow win;
	win.show();
	return app.exec();
}
