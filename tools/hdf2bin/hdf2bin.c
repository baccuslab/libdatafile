/* hdf2bin.c
 * A tool to convert recording files from HDF5 format to traditional 
 * *.bin-file binary format.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "hdf5.h"
#include "bintools.h"
#include "hdftools.h"

#define UL_PRE "\033[4m"
#define UL_POST "\033[0m"

const char PROGRAM_NAME[] = "hdf2bin";
const char AUTHOR[] = "Benjamin Naecker";
const char AUTHOR_EMAIL[] = "bnaecker@stanford.edu";
const char YEAR[] = "2015";
const char USAGE[] = "\n\
 Usage: hdf2bin [-v | --version] [-h | --help] [ -o " UL_PRE "outfile" UL_POST\
 " ] [ -s | --file-size " UL_PRE "size" UL_POST " ] " UL_PRE "infile" UL_POST "\n\
 Convert HDF5 recording files to traditional binary format.\n\n\
 Parameters:\n\
   " UL_PRE "size" UL_POST "\t\tMaximum size, in seconds, of data in a single\n\
   \t\toutput file. Data will be broken up into multiple files if needed.\n\
   \t\tDefault is 1000 seconds.\n\n\
   " UL_PRE "infile" UL_POST "\tThe source file to convert.\n\n\
   " UL_PRE "outfile" UL_POST "\tThe basename of the output files. If not given,\n\
   \t\tit uses the same basename as the input file.\n";
const unsigned int VERSION_MAJOR = 0;
const unsigned int VERSION_MINOR = 8;
const unsigned int DEFAULT_FILE_SIZE = 1000;

void print_usage_and_exit(void) {
	printf(USAGE);
	exit(EXIT_SUCCESS);
}

void print_version_and_exit(void) {
	printf("%s version %d.%d\n", PROGRAM_NAME, VERSION_MAJOR, VERSION_MINOR);
	printf("(C) %s %s %s\n", YEAR, AUTHOR, AUTHOR_EMAIL);
	exit(EXIT_SUCCESS);
}

char *make_outfile_basename(const char *infile_name) {
	char *period = strrchr(infile_name, '.');
	if (period == NULL)
		return infile_name;
	size_t length = period - infile_name + 1;
	char *outfile_name = calloc(length, sizeof(char));
	if (outfile_name == NULL)
		print_usage_and_exit();
	memcpy(outfile_name, infile_name, length - 1);
	return outfile_name;
}

unsigned int check_file_size_arg(char *sz) {
	if (!sz)
		return DEFAULT_FILE_SIZE;
	char *endptr = NULL;
	int ret = strtol(sz, &endptr, 10);
	if ( ((*endptr) != '\0') || (ret <= 0) ) {
		printf("Invalid file size: %s\n", sz);
		exit(EXIT_FAILURE);
	}
	return ret;
}

int main(int argc, const char *argv[]) {

	/* Parse input arguments */
	static struct option opts[] = {
		{ "version", 	no_argument, 		NULL, 'v' },
		{ "help", 		no_argument, 		NULL, 'h' },
		{ "output", 	required_argument, 	NULL, 'o' },
		{ "file-size", 	required_argument, 	NULL, 's' },
		{ NULL, 		0, 					NULL, 0 }
	};
	char *outfile_name = NULL;
	char *size_str = NULL;
	int opt;
	while ( (opt = getopt_long(argc, argv, "hvo:s:", opts, NULL)) != -1) {
		switch (opt) {
			case 'h':
				print_usage_and_exit();
			case 'v':
				print_version_and_exit();
			case 'o':
				outfile_name = optarg;
				break;
			case 's':
				size_str = optarg;
				break;
		}
	}
	unsigned int file_size = check_file_size_arg(size_str);
	unsigned int offset = 1;
	if (outfile_name && size_str)
		offset = optind;
	else if (outfile_name || size_str)
		offset = 3;
	argv += offset;
	argc -= offset;
	if (argc == 0)
		print_usage_and_exit();

	/* At this point, outfile_name points to a possible output file
	 * basename, and the first of any remaining input arguments will be 
	 * considered the input file
	 */
	char *infile_name = strdup(argv[0]);
	if (!outfile_name)
		outfile_name = make_outfile_basename(infile_name);

	/* Copy data from input HDF5 file to output file(s) */
	int ret = hdf2bin(infile_name, outfile_name, file_size);
	free(infile_name);
	free(outfile_name);
	return ret;
}

