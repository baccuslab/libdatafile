libdatafile
===========

`libdatafile` is a C++11 library for reading and writing data
to HDF5 files used to store data from MEA recordings in the Baccus Lab.

Dependencies and building
-------------------------

- C++11
- [HDF5](https://www.hdfgroup.org/HDF5/) with C++ bindings
- [Armadillo](https://www.arma.sourceforge.net) C++ linear algebra library

The library can be built by doing:

	$ qmake && make

On *nix systems, it is expected that headers for the HDF and Armadillo
libraries are in one of `/usr/include` or `/usr/local/include`, and the libraries
are in `/usr/lib` or `/usr/local/lib`. If that's not true, you'll need to 
move them or create links, or change the accompanying .pro file to match
your include paths.

