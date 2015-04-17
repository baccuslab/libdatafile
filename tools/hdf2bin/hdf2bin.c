/* hdf2bin.c
 * A tool to convert recording files from HDF5 format to traditional 
 * *.bin-file binary format.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <string.h>
#include <stdlib.h>

#include "hdf5.h"
#include "bintools.h"
#include "hdftools.h"

const char BIN_FILE_EXTENSION[] = ".bin";
const char USAGE[] = "\n\
 Usage: hdf2bin <infile> [<outfile>]\n\
 Convert HDF5 recording files to tradition binary format.\n\
 \n\
 Parameters:\n\
   <infile>\tThe source file to convert\n\
   <outfile>\tThe destination file name. If not given, it uses the same\n\
            \tbasename as the input file.\n";

void print_usage_and_exit(void) {
	printf(USAGE);
	exit(EXIT_FAILURE);
}

char *make_outfile_name(const char *infile_name) {
	char *period = strrchr(infile_name, '.');
	if ((period == NULL) || (period == infile_name))
		print_usage_and_exit();
	size_t base_length = period - infile_name;
	size_t length = base_length + 1 + strlen(BIN_FILE_EXTENSION);
	char *outfile_name = calloc(length, sizeof(char));
	if (outfile_name == NULL)
		print_usage_and_exit();
	memcpy(outfile_name, infile_name, base_length);
	memcpy(outfile_name + base_length, BIN_FILE_EXTENSION, 
			strlen(BIN_FILE_EXTENSION));
	return outfile_name;
}

int main(int argc, const char *argv[]) {
	/* Parse arguments */
	if ((argc == 1) || 
			((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help")) == 0))
		print_usage_and_exit();
	char *infile_name = strdup(argv[1]);
	char *outfile_name;
	if (argc == 2)
		outfile_name = make_outfile_name(infile_name);
	else
		outfile_name = strdup(argv[2]);
	int ret = hdf2bin(infile_name, outfile_name);
	free(infile_name);
	free(outfile_name);
	return ret;
}
