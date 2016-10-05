libdatafile
===========

`libdatafile` is a C++11 library for reading and writing data in the various
HDF5-based file formats.

(C) 2016 Benjamin Naecker bnaecker@stanford.edu

File formats
------------

The library contains C++ classes for interacting with data in 2 types of files.

1.	Raw voltage data from MEA recordings.
2.	"Snippet" files, which contain information about extracted candidate spike snippets.

Example
-------

	/* Create or open the data file. If the file exists, it will be opened read-only. */
	DataFile df("filename.h5");

	if (df.array().find("hidens") == std::string::npos) {
		std::cout << "Data file represents a multi-channel systems recording";
	} else {
		std::cout << "Data file represents a HiDens recording";
	}

	/* Assign data to a portion of the file. This will throw a std::logic_error
	 * if the file was opened read-only.
	 */
	int nsamples = 100;
	arma::mat data(nsamples, df.nchannels(), arma::fill::randn);
	df.setData(0, nsamples, data);

	/* Data can be read out as many different types */
	arma::vec channel = df.data(5, 0, 1000); // read first 1000 samples of channel 5
	arma::Mat<int16_t> moreData;
	df.data(0, 50, moreData); // read first 50 samples of all channels into `moreData`

	/* Read the configuration from an existing HiDens recording.
	 * The Configuration data type is defined in libmea-device/include/configuration.h.
	 */
	HidensFile hf("hidens-datafile.h5");
	Configuration config = hf.configuration(); 

Column- vs. row-major
---------------------

The HDF5 library used to manage the underlying data files stores data in row-major
ordering. This means that the last index varies the fastest. Datasets containing
raw voltage data are stored shaped as (nchannels, nsamples), meaning that all
samples from one channel are grouped together, followed by all samples from the 
next channel, etc.

(This isn't strictly true because of how the HDF5 library chunks its files, but
it is conceptually correct.)

However, `libdatafile` uses an excellent C++ linear algebra library called 
Armadillo to manipulate data in memory. This library uses column-major ordering.
To avoid copying and transposing data all over the place, `libdatafile` simply
stores the data in memory as if it were transposed.

That is, a chunk of data in row-major (HDF5) ordering of shape (nchannels, nsamples)
has an identical memory layout to a chunk of data in column-major ordering with
shape (nsamples, nchannels). There's nothing tricky here, but it can get confusing
to remember which library uses which and that there is no explicit transposing
happening between disk and memory.

The documenation to the `data()` and `setData()` functions clearly lay out the
shape of the matrices passed and returned.

Dependencies and building
-------------------------

- C++11
- [HDF5](https://www.hdfgroup.org/HDF5/) with C++ bindings
- [Armadillo](http://arma.sourceforge.net) C++ linear algebra library
- [Qt 5.3 or later](http://qt.io). The project uses Qt's `qmake` build system for 
compilation, and its testing framework to run unit tests.

The library can be built by doing:

	$ qmake && make

The accompanying `.pro` file makes an attempt to find the headers and libraries
for both HDF5 and Armadillo. If compilation fails because either the header or
libraries cannot be found, add the location of these files to the `.pro` file,
under the `INCLUDEPATH` and `LIBS` variables, respectively.

Tests and documentation
-----------------------

Documentation for the library is created via [Doxygen](http://doxygen.org). It can be
built by simply running `doxygen` inside the main directory.

Tests are run using Qt's test framework. Compile and run the tests by doing:
	$ cd tests/
	$ qmake && make check

If a test fails, please create an issue for the repository.

