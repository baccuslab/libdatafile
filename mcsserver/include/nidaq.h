/* nidaq.h
 *
 * Header file defining class for interfacing with National Instruments
 * data acquisition device itself.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef DAQSRV_NIDAQ_H_
#define DAQSRV_NIDAQ_H_

#include <QList>

namespace nidaq {
QList<double> ADC_RANGE_LIST = {0.5, 1, 2, 5, 10};

class Task {
};

};

#endif

