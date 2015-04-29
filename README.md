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
- [Armadillo C++ linear algebra library] (http://arma.sourceforge.net)
- [Google Protocol Buffers] (https://developers.google.com/protocol-buffers)

Building
========

The `mearec` application suite uses `qmake` for its build environment, even for
those components that are not written using Qt. Each component has its own
`.pro` file, and can be built independently as:

	$ cd <component>
	$ qmake5 && make

The entire project can be built, either in debug or release mode, by just calling
`qmake5` in the top `mearec` directory.
