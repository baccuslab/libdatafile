/* main.cpp
 * Entry point for mealog application 
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <cstdlib>
#include <QApplication>
#include <QString>
#include <QStringList>
#include <QInputDialog>
#include "mealogwindow.h"

int main(int argc, char *argv[]) {

	/* Setup application */
	QApplication app(argc, argv);
	app.setOrganizationName("baccuslab");
	app.setApplicationName("mealog");

	/* Ask user which array is being recorded from */
	bool ok;
	QString array = QInputDialog::getItem(
			0, "Choose array",
			"Choose an array type from which to record data",
			QStringList({"HiDens", "Multichannel systems"}),
			0, false, &ok);
	if (!ok) {
		app.quit();
		exit(0);
	}

	MealogWindow win(array);
	win.show();
	return app.exec();
}
