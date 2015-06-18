/* bin2hdf.c
 * A tool to convert a traditional *.bin-file to a newer HDF5 format file.
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

const char PROGRAM_NAME[] = "bin2hdf";
const char AUTHOR[] = "Benjamin Naecker";
const char AUTHOR_EMAIL[] = "bnaecker@stanford.edu";
const char YEAR[] = "2015";
const char USAGE[] = "\n\
 Usage: bin2hdf [-h | --help] [-v | --version] [-o " UL_PRE "outfile" UL_POST\
 " ] " UL_PRE "infile" UL_POST " [ " UL_PRE "infile" UL_POST " ... ]\n\
 Convert traditional bin files to HDF5 format.\n\
 \n\
 Parameters:\n\
   " UL_PRE "infile" UL_POST "\tThe source file(s) to convert. Multiple files\n\
   \t\tare joined into a single HDF5 output file.\n\n\
   " UL_PRE "outfile" UL_POST "\tThe destination file name. If not given, the base\n\
   \t\tname of the first input file is used.\n";
const unsigned int VERSION_MAJOR = 0;
const unsigned int VERSION_MINOR = 8;

void print_usage_and_exit(void) {
	printf(USAGE);
	exit(EXIT_SUCCESS);
}

void print_version_and_exit(void) {
	printf("%s version %d.%d\n", PROGRAM_NAME, VERSION_MAJOR, VERSION_MINOR);
	printf("(C) %s %s %s\n", YEAR, AUTHOR, AUTHOR_EMAIL);
	exit(EXIT_SUCCESS);
}

char *make_outfile_name(const char *infile_name) {
	char *period = strrchr(infile_name, '.');
	if (period == NULL)
		period = (char *) infile_name + strlen(infile_name);
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

char **copy_input_files(const char *argv[], int num_files) {
	char **names = calloc(num_files, sizeof(char *));
	if (!names) {
		perror("Could not copy input filenames");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < num_files; i++)
		names[i] = strdup(argv[i]);
	return names;
}

void free_infile_names(int num_files, char **names) {
	for (int i = 0; i < num_files; i++)
		free(names[i]);
	free(names);
}

int main(int argc, const char *argv[]) {

	/* Parse input arguments */
	static struct option opts[] = {
		{ "version", 	no_argument, 		NULL, 'v' },
		{ "help", 		no_argument, 		NULL, 'h' },
		{ "output", 	required_argument, 	NULL, 'o' }, 
		{ NULL, 		0, 					NULL, 0 }
	};
	char *outfile_name = NULL;
	int opt;
	while ( (opt = getopt_long(argc, argv, "hvo:", opts, NULL)) != -1) {
		switch (opt) {
			case 'h':
				print_usage_and_exit();
			case 'v':
				print_version_and_exit();
			case 'o':
				outfile_name = optarg;
				break;
		}
	}
	int offset = outfile_name ? optind : 1;
	argv += offset;
	argc -= offset;
	if (argc == 0)
		print_usage_and_exit();

	/* At this point, outfile_name points to a possible output file name, 
	 * and any remaining arguments are input bin files.
	 */
	int num_input_files = argc;
	char **infile_names = copy_input_files(argv, num_input_files);
	if (!outfile_name)
		outfile_name = make_outfile_name(infile_names[0]);

	/* Copy data from input file(s) to output HDF5 file */
	int ret = bin2hdf(num_input_files, infile_names, outfile_name);
	if (ret)
		exit(EXIT_FAILURE);
	free(outfile_name);
	free_infile_names(num_input_files, infile_names);
	return ret;
}

