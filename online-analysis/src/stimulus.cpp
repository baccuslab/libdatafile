/* stimulus.cpp
 * ------------
 *
 * Implementation of class representing a stimulus in online analyses.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <cmath>
#include "stimulus.h"

Stimulus::Stimulus(std::string fname, std::string stimname)
{
	/* Verify file */
	filename = fname;
	if ( !H5::H5File::isHdf5(filename) ) {
		std::cerr << "Invalid HDF5 file: " + filename;
		throw std::invalid_argument("Invalid HDF5 file");
	}
	file = H5::H5File(filename, H5F_ACC_RDONLY);

	/* Open stimulus and verify */
	try {
		stimulus = file.openDataSet(stimname);
		stimSpace = stimulus.getSpace();
		H5::Attribute attr = stimulus.openAttribute("frame-rate");
		attr.read(attr.getDataType(), &trueRate);
	} catch ( ... ) {
		std::cerr << "Invalid stimulus file";
		throw std::invalid_argument("Stimulus file is invalid");
	}
	frate = std::round(trueRate);
	ifi = 1.0 / frate;

	/* Get sizes */
	ndim_ = stimSpace.getSimpleExtentNdims();
	dims = new hsize_t[ndim_];
	stimSpace.getSimpleExtentDims(dims);
}

Stimulus::~Stimulus(void) 
{
	try {
		file.close();
	} catch (H5::FileIException &e) { }
	delete[] dims;
}

hsize_t Stimulus::nx(void)
{
	if ( ndim_ > 1 )
		return dims[0];
	return 1;
}

hsize_t Stimulus::ny(void)
{
	if ( ndim_ > 2 )
		return dims[1];
	return 1;
}

hsize_t Stimulus::nframes(void)
{
	return dims[ndim_ - 1];
}

hsize_t Stimulus::frameBefore(double t)
{
	if (t <= ifi)
		return 0;
	hsize_t val = std::floor(t / ifi);
	return (val > nframes()) ? nframes() : val;
}

arma::mat Stimulus::frame(unsigned int frame)
{
	/* Initialize size arrays */
	hsize_t *space_offset = new hsize_t[ndim_]();
	space_offset[ndim_ - 1] = frame;
	hsize_t *space_count = new hsize_t[ndim_];
	for (auto i = 0; i < ndim_ - 1; i++)
		space_count[i] = dims[i];
	space_count[ndim_ - 1] = 1;

	/* Select data set */
	stimSpace.selectHyperslab(H5S_SELECT_SET, space_count, space_offset);
	if ( !stimSpace.selectValid() ) {
		std::cerr << "Stimulus frame selection invalid:" << std::endl;
		std::cerr << "Offset: ( " << space_offset << " )" << std::endl;
		std::cerr << "Count: ( " << space_count << " )" << std::endl;
		throw H5::DataSpaceIException("Invalid stimulus selection");
	}

	/* Create memory array and selection */
	hsize_t *memdims = new hsize_t[ndim_];
	for (auto i = 0; i < ndim_ - 1; i++) 
		memdims[i] = dims[i];
	memdims[ndim_ - 1] = 1;
	H5::DataSpace memspace(ndim_, memdims);
	hsize_t *mem_offset = new hsize_t[ndim_]();
	hsize_t *mem_count = new hsize_t[ndim_];
	for (auto i = 0; i < ndim_ - 1; i++) 
		mem_count[i] = dims[i];
	mem_count[ndim_ - 1] = 1;
	memspace.selectHyperslab(H5S_SELECT_SET, mem_count, mem_offset);
	if ( !memspace.selectValid() ) {
		std::cerr << "Memory dataspace selection invalid:" << std::endl;
		std::cerr << "Offset: ( " << space_offset << " )" << std::endl;
		std::cerr << "Count: ( " << space_count << " )" << std::endl;
		throw H5::DataSpaceIException("Invalid stimulus selection");
	}
	arma::mat out(nx(), ny());
	stimulus.read(out.memptr(), H5::PredType::IEEE_F64LE, memspace, stimSpace);
	return out.t();
}

void _swap_order(double *data, hsize_t m, hsize_t n, hsize_t p)
{
	auto M = n * p;
	auto N = m * n;
	for (auto i = 0; i < m; i++) {
		for (auto j = 0; j < n; j++) {
			for (auto k = 0; k < p; k++)
				data[ i + m * j + N * k ] = data[ M * i + p * j + k ];
		}
	}
}

void Stimulus::frames_(unsigned int start, unsigned int end, double *buffer)
{
	/* Initialize size arrays */
	hsize_t total_nframes = end - start;
	hsize_t *space_offset = new hsize_t[ndim_]();
	space_offset[ndim_ - 1] = start;
	hsize_t *space_count = new hsize_t[ndim_]();
	for (auto i = 0; i < ndim_ - 1; i++)
		space_count[i] = dims[i];
	space_count[ndim_ - 1] = total_nframes;

	/* Select data hyperslab */
	stimSpace.selectHyperslab(H5S_SELECT_SET, space_count, space_offset);
	if ( !stimSpace.selectValid() ) {
		std::cerr << "Stimulus frame selection invalid:" << std::endl;
		std::cerr << "Offset: ( " << space_offset << " )" << std::endl;
		std::cerr << "Count: ( " << space_count << " )" << std::endl;
		throw H5::DataSpaceIException("Invalid stimulus selection");
	}

	/* Describe memory space */
	hsize_t *memdims = new hsize_t[ndim_]();
	for (auto i = 0; i < ndim_ - 1; i++) 
		memdims[i] = dims[i];
	memdims[ndim_ - 1] = total_nframes;
	H5::DataSpace memspace(ndim_, memdims);
	hsize_t *mem_offset = new hsize_t[ndim_](); // leave as all zeros
	hsize_t *mem_count = new hsize_t[ndim_]();
	for (auto i = 0; i < ndim_ - 1; i++) 
		mem_count[i] = dims[i];
	mem_count[ndim_ - 1] = total_nframes;
	memspace.selectHyperslab(H5S_SELECT_SET, mem_count, mem_offset);
	if ( !memspace.selectValid() ) {
		std::cerr << "Memory dataspace selection invalid:" << std::endl;
		std::cerr << "Offset: ( " << space_offset << " )" << std::endl;
		std::cerr << "Count: ( " << space_count << " )" << std::endl;
		throw H5::DataSpaceIException("Invalid stimulus selection");
	}

	/* Read */
	stimulus.read(buffer, H5::PredType::IEEE_F64LE, memspace, stimSpace);
}

arma::cube Stimulus::frames(unsigned int start, unsigned int end)
{
	if ( end <= start )
		throw std::logic_error("Invalid selection: end frame (" +
				std::to_string(end) + ") must be greater than start (" +
				std::to_string(start) + ")");
	hsize_t total_nframes = end - start;

	/* Read data and swap from row to column major */
	arma::cube out(nx(), ny(), total_nframes);
	frames_(start, end, out.memptr());
	_swap_order(out.memptr(), nx(), ny(), total_nframes);
	return out;
}

void Stimulus::frames(unsigned int start, unsigned int end, arma::vec& out)
{
	if ( end <= start )
		throw std::logic_error("Invalid selection: end frame (" +
				std::to_string(end) + ") must be greater than start (" +
				std::to_string(start) + ")");
	hsize_t total_nframes = end - start;

	/* Read data and swap from row to column major */
	out.set_size(total_nframes);
	frames_(start, end, out.memptr());
}

void Stimulus::frames(unsigned int start, unsigned int end, arma::mat& out)
{
	if ( end <= start )
		throw std::logic_error("Invalid selection: end frame (" +
				std::to_string(end) + ") must be greater than start (" +
				std::to_string(start) + ")");
	hsize_t total_nframes = end - start;

	/* Read data and swap from row to column major */
	out.set_size(total_nframes, nx());
	frames_(start, end, out.memptr());
	out = out.t();
}

void Stimulus::frames(unsigned int start, unsigned int end, arma::cube& out)
{
	if ( end <= start )
		throw std::logic_error("Invalid selection: end frame (" +
				std::to_string(end) + ") must be greater than start (" +
				std::to_string(start) + ")");
	hsize_t total_nframes = end - start;

	/* Read data and swap from row to column major */
	out.set_size(nx(), ny(), total_nframes);
	frames_(start, end, out.memptr());
	_swap_order(out.memptr(), nx(), ny(), total_nframes);
}

void Stimulus::at(double startTime, double rate, unsigned int N, arma::vec& out)
{
	auto fs = 1 / rate;
	auto first = frameBefore(startTime);
	auto last = frameBefore(startTime + fs * (N - 1)) + 1;
	arma::vec base;
	frames(first, last, base);
	out.set_size(N);
	for (auto i = 0; i < N; i++) 
		out(i) = base(frameBefore(startTime + fs * i) - first);
}

void Stimulus::at(double startTime, double rate, unsigned int N, arma::mat& out)
{
	auto fs = 1 / rate;
	auto first = frameBefore(startTime);
	auto last = frameBefore(startTime + fs * (N - 1)) + 1;
	arma::mat base;
	frames(first, last, base);
	out.set_size(base.n_rows, N);
	for (auto i = 0; i < N; i++)
		out.col(i) = base.col(frameBefore(startTime + fs * i) - first);
}

void Stimulus::at(double startTime, double rate, unsigned int N, arma::cube& out)
{
	auto fs = 1 / rate;
	auto first = frameBefore(startTime);
	auto last = frameBefore(startTime + fs * (N - 1)) + 1;
	arma::cube base;
	frames(first, last, base);
	out.set_size(base.n_rows, base.n_cols, N);
	for (auto i = 0; i < N; i++)
		out.slice(i) = base.slice(frameBefore(startTime + fs * i) - first);
}

void Stimulus::at(const arma::vec& points, arma::vec& out)
{
	auto first = frameBefore(points(0));
	auto last = frameBefore(points(points.n_elem - 1));
	arma::vec base;
	frames(first, last, base);
	out.set_size(points.n_elem);
	for (auto i = 0; i < points.n_elem; i++)
		out(i) = base(frameBefore(points(i)));
}

void Stimulus::at(const arma::vec& points, arma::mat& out)
{
	auto first = frameBefore(points(0));
	auto last = frameBefore(points(points.n_elem - 1));
	arma::vec base;
	frames(first, last, base);
	out.set_size(base.n_rows, points.n_elem);
	for (auto i = 0; i < points.n_elem; i++)
		out.col(i) = base.col(frameBefore(points(i)));
}

void Stimulus::at(const arma::vec& points, arma::cube& out)
{
	auto first = frameBefore(points(0));
	auto last = frameBefore(points(points.n_elem - 1));
	arma::cube base;
	frames(first, last, base);
	out.set_size(base.n_rows, base.n_cols, points.n_elem);
	for (auto i = 0; i < points.n_elem; i++)
		out.slice(i) = base.slice(frameBefore(points(i)));
}

