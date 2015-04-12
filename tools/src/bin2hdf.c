/* bin2hdf.c
 * A tool to convert a traditional *.bin-file to a newer HDF5 format file.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <string.h>
#include <stdlib.h>

#include "hdf5.h"
#include "bintools.h"
#include "hdftools.h"

const char H5_FILE_EXTENSION[] = ".h5";
const char USAGE[] = "\n\
 Usage: bin2hdf <infile> [<outfile>]\n\
 Convert traditional bin files to HDF5 format.\n\
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
	size_t length = base_length + 1 + strlen(H5_FILE_EXTENSION);
	char *outfile_name = calloc(length, sizeof(char));
	if (outfile_name == NULL)
		print_usage_and_exit();
	memcpy(outfile_name, infile_name, base_length);
	memcpy(outfile_name + base_length, H5_FILE_EXTENSION, 
			strlen(H5_FILE_EXTENSION));
	return outfile_name;
}

int main(int argc, const char *argv[]) {
	/* Parse arguments */
	if ((argc == 1) || 
			((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)))
		print_usage_and_exit();
	char *infile_name = strdup(argv[1]);
	char *outfile_name;
	if (argc == 2)
		outfile_name = make_outfile_name(infile_name);
	else
		outfile_name = strdup(argv[2]);
	int ret = bin2hdf(infile_name, outfile_name);
	if (ret)
		exit(EXIT_FAILURE);
	free(outfile_name);
	free(infile_name);
	return ret;
}
