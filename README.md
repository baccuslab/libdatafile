mearec
======

The `mearec` library is a set of applications and tools for recording data from
the Multichannel Systems arrays using the National Instruments USB-6225 in the
Baccus Lab.

Components
==========

- `meaview` - Play data recordings, either live or pre-recorded
- `mealog` - Create and save experiments, logging the data in HDF5 files
- `daq-client` - A client interface to the `daqsrv` remote NI-DAQ server
- `h5recording` - A class for reading and recording data to HDF5 files
- `tools` - Miscellaneous tools

Dependencies
============

- C++11
- [Qt5] (http://doc.qt.io/qt-5/index.html)
- [HDF5 with C++ bindings] (http://hdfgroup.org/HDF5/)
- [Boost] (http://www.boost.org/)

