/* files.cpp
 * Implementation of various file readers/writers for meaview application.
 * (C) Benjamin Naecker bnaecker@stanford.edu
 */

/* C++ includes */

/* Qt includes */

/* meaview includes */
#include "files.h"

BinFile::BinFile(
		const QString &name, 
		QObject *parent) :
	QFile(name, parent) {
}


