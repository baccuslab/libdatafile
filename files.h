/* files.h
 * Header file for various file readers/writers used in the meaview application.
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _FILES_H_
#define _FILES_H_

/* C++ includes */

/* Qt includes */
#include <QFile>

/* meaview includes */
#include "config.h"


/* class: BinFile
 * --------------
 * The BinFile class represents data as it should be written directly
 * to disk in the standard AIB format.
 */
class BinFile : public QFile {
	Q_OBJECT
};

#endif

