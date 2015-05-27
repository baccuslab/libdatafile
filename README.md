mearec
======

The `mearec` library is a set of applications, libraries, and tools for 
recording data from the Multichannel Systems arrays in the Baccus Lab.

(C) 2015 Benjamin Naecker bnaecker@stanford.edu

Components
==========

- `mealog` - Record data or play back old data recordings.
- `daqsrv` - A small server application that interfaces with our National
Instruments data acquisition device, and serves data over the network.
- `daqclient` - A client interface to the `daqsrv` remote NI-DAQ server.
- `h5recording` - A class for reading and recording data to HDF5 files.
- `messaging`/`mealogclient` - The Mealog application provides information 
about the status of its recording though simple message-based network protocol. 
The messaging protocol and a C++ client library are defined here. (This is 
not yet fully implemented)
- `tools` - Miscellaneous tools, such as converting HDF5 recordings to traditional
bin files and vice-versa.


Dependencies
============

The `mearec` suite has been tested on Mac OS X 10.9 and 10.10, as well as 
Ubuntu 14.10.

- C++11 or greater
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
(numSamples, numChannels). This doesn't mean data is transposed, because of
the different ordering formats of the two libraries. Thus it's not a source
of inefficiency, just possible programming confusion.

Attributions
============

Most code here is copyrighted by Benjamin Naecker (bnaecker@stanford.edu), with
one very notable exception.

`QCustomPlot` is the library used for creating plots of data. This code is
copyrighted by Emanuel Eichhammer, whose contact info can be found 
[here](http://www.qcustomplot.com/index.php/contact).

