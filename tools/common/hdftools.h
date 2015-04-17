/* hdftools.h
 * Public API for tools to manipulate HDF5 files used in Baccus lab
 * MEA recordings. This includes functions to convert from HDF5 to 
 * traditional bin files.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _HDFTOOLS_H_
#define _HDFTOOLS_H_

/* Convert traditional binary file to new HDF5 format */
int bin2hdf(char *binfile, char *hdf_file);

/* Convert new HDF5 format file to traditional binary format */
int hdf2bin(char *hdf_file, char *binfile);

#endif
