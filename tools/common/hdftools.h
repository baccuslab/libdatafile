/* hdftools.h
 * Public API for tools to manipulate HDF5 files used in Baccus lab
 * MEA recordings. This includes functions to convert from HDF5 to 
 * traditional bin files.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _HDFTOOLS_H_
#define _HDFTOOLS_H_

#include <stdbool.h>

const char BIN_FILE_EXTENSION[] = ".bin";
const char H5_FILE_EXTENSION[] = ".h5";
const char ISO_FORMAT[] = "%Y-%m-%dT%H:%M:%S";
const size_t ISO_FORMAT_LEN = 19;

/* Convert traditional binary file(s) to new HDF5 format */
int bin2hdf(int num_infiles, char **binfiles, char *hdf_file);

/* Convert new HDF5 format file to traditional binary format */
int hdf2bin(char *hdf_file, char *binfile, unsigned int file_size, bool strict);

#endif
