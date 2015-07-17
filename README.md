MeaRec
======

The `MeaRec` library is a set of tools, libraries, and a GUI application
for recording multi-electrode array data from either Multichannel Systems
arrays, or the HiDens system from the Hierlemann group at ETH Basel.

(C) 2015 Benjamin Naecker bnaecker@stanford.edu

Components
==========

- `mealog`
	- The main application component of the `MeaRec` suite. A GUI application
	for creating new recordings or playing back old recordings, and visualizing
	the resulting data.
- `daqsrv` - A small server application that interfaces with our National
Instruments data acquisition device, and serves data over the network.
- `dataclient` 
	- A client library for talking with and receiving data from the data
	source servers
		- `McsClient` A client to the `daqsrv` program collecting data
		from the MCS array system
		- `HidensClient` A client to the HiDens data server
- `h5recording` 
	- A class for reading and recording data to HDF5 files.
	- `HidensRecording` and `McsRecording` subclasses
- `online-analysis`
	- Classes and functionality for implementing online analysis of
	data recordings
- `messaging` 
	- The Mealog application provides information about the status of its 
	recording though simple message-based network protocol. The messaging 
	protocol and a C++ client library are defined here. (This is not yet 
	fully implemented)
- `tools` - Miscellaneous tools
	- `hdf2bin`/`bin2hdf` - Convert between HDF5 recordings and AIB binary format
	- `fifo2hdf` - Convert from Igor FIFO files to HDF5 recordings (not yet implemented)


Dependencies
============

The `mearec` suite has been tested on Mac OS X 10.9 and 10.10, as well as 
Ubuntu 14.10.

- C++11 or greater
- [Qt5] (http://doc.qt.io/qt-5/index.html)
- [HDF5 (>= 1.8.12 with C++ bindings] (http://hdfgroup.org/HDF5/)
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

Data files
==========

A quick note about data file formats. Data is stored in HDF5 format, and the
`mearec` suite includes tools to convert old-school binary files to HDF5 and
back. However, data is stored in memory using the Armadillo C++ linear algebra
library's matrices, since the format is efficient, convenient, and allows 
easy online computations if desired.

This presents a bit of a weird situation, because HDF5 uses row-major data 
ordering, while Armadillo uses column-major. This means data is stored on
disk as a dataset with shape (numChannels, numSamples), but in memory as
(numSamples, numChannels). This allows each library to have access to the data
in its most efficient stride pattern, without transposing any data. It is 
not inefficient or wrong, but may lead to programming or debugging confusion.

Attributions
============

Most code here is copyrighted by Benjamin Naecker (bnaecker@stanford.edu), with
one very notable exception.

`QCustomPlot` is the library used for creating plots of data. This code is
copyrighted by Emanuel Eichhammer, whose contact info can be found 
[here](http://www.qcustomplot.com/index.php/contact). The work here is 
greatly indebted to this wonderful library.

