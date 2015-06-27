/* hdftools.c
 * Implementation of library functions for converting between bin files
 * and HDF5 files
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "hdf5.h"
#include "hdftools.h"
#include "bintools.h"

#define RANK 2

/* Helper function prototypes */
FILE **open_input_files(int, char **);
bin_hdr_t **read_bin_headers(int, FILE **);
void free_bin_headers(int, bin_hdr_t **);
void close_files(int, FILE **);
int write_dataset_attrs(hid_t, bin_hdr_t *);
int write_hdf_attr(hid_t, char *, hid_t, hid_t, void *);
int write_hdf_string_attr(hid_t, char *, char *, hid_t);
int write_recording_attrs(hid_t, bin_hdr_t *);
int write_data_to_hdf(hid_t, hid_t, int, bin_hdr_t **, FILE **);

int write_data_to_bin(hid_t, hid_t, unsigned int, bin_hdr_t **, FILE **);
unsigned int *compute_num_bin_files(hid_t, unsigned int, unsigned int *);
bin_hdr_t **create_bin_headers(hid_t, unsigned int, unsigned int *, bool);
int read_hdf_attr(hid_t, char *, hid_t, void *);
int read_hdf_string_attr(hid_t, char *, uint32_t *, char **);
int write_bin_headers(unsigned int, bin_hdr_t **, FILE **);
char *create_outfile_name(char *, unsigned int);
FILE **open_outfiles(char *, unsigned int);

int bin2hdf(int num_files, char **infiles, char *outfile) {

	/* Open the source bin files and read their headers */
	FILE **fp_list = open_input_files(num_files, infiles);
	bin_hdr_t **hdr_list = read_bin_headers(num_files, fp_list);

	/* Identifiers and attributes for the HDF5 output file */
	hid_t file_id, dset_id, dspace_id, dset_create_props;
	hsize_t total_samples = 0;
	for (int i = 0; i < num_files; i++)
		total_samples += hdr_list[i]->nsamples;
	hsize_t dims[RANK] = {hdr_list[0]->nchannels, total_samples};

	/* Open the output file. No compression for now */
	if ((file_id = H5Fcreate(outfile, H5F_ACC_TRUNC, 
			H5P_DEFAULT, H5P_DEFAULT)) < 0) {
		printf("Could not open output file\n");
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		return -1;
	}

	/* Create property list giving chunked data */
	if ((dset_create_props = H5Pcreate(H5P_DATASET_CREATE)) < 0) {
		printf("Could not create dataset properties\n");
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		H5Fclose(file_id);
		return -1;
	}
	hsize_t chunk_size[RANK] = {hdr_list[0]->nchannels, 
			hdr_list[0]->block_size};
	if ((H5Pset_chunk(dset_create_props, RANK, chunk_size)) < 0) {
		printf("Could not set dataset properties\n");
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		H5Fclose(file_id);
		return -1;
	}
	
	/* Create actual dataspace and dataset */
	if ((dspace_id = H5Screate_simple(RANK, dims, NULL)) < 0) {
		printf("Could not create dataspace\n");
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		H5Fclose(file_id);
		H5Pclose(dset_create_props);
		return -1;
	}
	if ((dset_id = H5Dcreate2(file_id, "data", 
			H5T_STD_I16LE, dspace_id, 
			H5P_DEFAULT, dset_create_props, H5P_DEFAULT)) < 0) {
		printf("COuld not create dataset\n");
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		H5Fclose(file_id);
		H5Pclose(dset_create_props);
		return -1;
	}

	/* Write the information in the header into the attributes of the dataset */
	if (write_dataset_attrs(dset_id, hdr_list[0])) {
		printf("Could not write dataset attributes\n");
		H5Sclose(dspace_id);
		H5Dclose(dset_id);
		H5Fclose(file_id);
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		return -1;
	}

	/* Write recording status metadata as attributes of the file */
	if (write_recording_attrs(file_id, hdr_list[0])) {
		printf("Could not write recording attributes\n");
		H5Sclose(dspace_id);
		H5Dclose(dset_id);
		H5Fclose(file_id);
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		return -1;
	}

	/* Write the data itself */
	if (write_data_to_hdf(dset_id, dspace_id, num_files, hdr_list, fp_list)) {
		printf("Could not write data\n");
		H5Sclose(dspace_id);
		H5Dclose(dset_id);
		H5Fclose(file_id);
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		return -1;
	}

	/* Close up shop */
	if (H5Sclose(dspace_id)) {
		H5Dclose(dset_id);
		H5Fclose(file_id);
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		return -1;
	}
	if (H5Dclose(dset_id)) {
		H5Fclose(file_id);
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		return -1;
	}
	if (H5Fclose(file_id)) {
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		return -1;
	}
	free_bin_headers(num_files, hdr_list);
	close_files(num_files, fp_list);

	return 0;
}

int hdf2bin(char *infile, char *outfile, unsigned int file_size, bool strict) {

	/* Open the source HDF5 file */
	hid_t file_id, dset_id, dspace_id;
	if ((file_id = H5Fopen(infile, H5F_ACC_RDONLY, H5P_DEFAULT)) < 0) {
		if (H5Fis_hdf5(infile) == 0)
			printf("Invalid HDF5 file: %s\n", infile);
		else
			printf("Could not open input file\n");
		return -1;
	}

	/* Open data set and get its dataspace */
	if ((dset_id = H5Dopen(file_id, "data", H5P_DEFAULT)) < 0) {
		printf("Could not open dataset\n");
		H5Fclose(file_id);
		return -1;
	}
	if ((dspace_id = H5Dget_space(dset_id)) < 0) {
		printf("Could not open dataspace\n");
		H5Fclose(file_id);
		H5Dclose(dset_id);
		return -1;
	}

	/* Determine number of bin files needed for all data */
	unsigned int num_files = 0;
	unsigned int *file_sizes = compute_num_bin_files(dset_id, 
			file_size, &num_files);

	/* Create a list of corresponding headers for the bin file(s) */
	bin_hdr_t **hdr_list = create_bin_headers(dset_id, 
			num_files, file_sizes, strict);
	if (hdr_list == NULL) {
		printf("Could not create bin headers\n");
		H5Fclose(file_id);
		H5Dclose(dset_id);
		H5Sclose(dspace_id);
		return -1;
	}

	/* Open the bin file and write the header */
	FILE **fp_list = open_outfiles(outfile, num_files);
	if (fp_list == NULL) {
		printf("Could not open output file: %s\n", outfile);
		H5Fclose(file_id);
		H5Dclose(dset_id);
		H5Sclose(dspace_id);
		free_bin_headers(num_files, hdr_list);
		return -1;
	}
	if (write_bin_headers(num_files, hdr_list, fp_list)) {
		printf("Could not write bin header\n");
		H5Fclose(file_id);
		H5Dclose(dset_id);
		H5Sclose(dspace_id);
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		return -1;
	}

	/* Write the HDF5 dataset to the file */
	if (write_data_to_bin(dset_id, dspace_id, num_files, hdr_list, fp_list)) {
		printf("Could not write bin data\n");
		H5Fclose(file_id);
		H5Dclose(dset_id);
		H5Sclose(dspace_id);
		free_bin_headers(num_files, hdr_list);
		close_files(num_files, fp_list);
		return -1;
	}

	return 0;
}

FILE **open_input_files(int num_files, char **names) {
	FILE **fp = calloc(num_files, sizeof(FILE *));
	if (!fp) {
		perror("Could not open input files");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < num_files; i++) {
		if ( (fp[i] = fopen(names[i], "r")) == NULL) {
			perror("Could not open input file");
			exit(EXIT_FAILURE);
		}
	}
	return fp;
}

bin_hdr_t **read_bin_headers(int num_files, FILE **fp) {
	bin_hdr_t **hdrs = calloc(num_files, sizeof(bin_hdr_t *));
	if (!hdrs) {
		perror("Could not read bin file headers");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < num_files; i++) {
		if ( !(hdrs[i] = read_bin_header(fp[i])) ) {
			perror("Could not read bin file header");
			exit(EXIT_FAILURE);
		}
	}
	return hdrs;
}

void free_bin_headers(int num_files, bin_hdr_t **hdrs) {
	for (int i = 0; i < num_files; i++)
		free_bin_header(hdrs[i]);
	free(hdrs);
}

void close_files(int num_files, FILE **fp) {
	for (int i = 0; i < num_files; i++)
		fclose(fp[i]);
	free(fp);
}

int write_hdf_attr(hid_t dset_id, char *name, 
		hid_t dtype, hid_t dspace_id, void *buffer) {
	hid_t attr_id = H5Acreate(dset_id, name, dtype, 
			dspace_id, H5P_DEFAULT, H5P_DEFAULT);
	if (attr_id < 0)
		return -1;
	if (H5Awrite(attr_id, dtype, buffer) < 0)
		return -1;
	return H5Aclose(attr_id);
}

int write_hdf_string_attr(hid_t dset_id, char *name, 
		char *value, hid_t space_id) {
	hid_t string_type = H5Tcopy(H5T_C_S1);
	if (string_type < 0)
		return -1;
	if (H5Tset_size(string_type, strlen(value)) < 0)
		return -1;
	hid_t attr_id = H5Acreate(dset_id, name, string_type, space_id,
			H5P_DEFAULT, H5P_DEFAULT);
	if (attr_id < 0)
		return -1;
	if (H5Awrite(attr_id, string_type, value) < 0)
		return -1;
	return H5Aclose(attr_id);
}

int write_dataset_attrs(hid_t dset_id, bin_hdr_t *hdr) {
	hsize_t dims[1] = {1};
	hid_t space_id = H5Screate_simple(1, dims, NULL); // Scalar

	/* Type */
	if (write_hdf_attr(dset_id, "bin-file-type", H5T_STD_I16LE, 
			space_id, &(hdr->type)) < 0)
		return -1;

	/* Version */
	if (write_hdf_attr(dset_id, "bin-file-version", H5T_STD_I16LE,
				space_id, &(hdr->version)) < 0)
		return -1;

	/* Sample rate */
	if (write_hdf_attr(dset_id, "sample-rate", H5T_IEEE_F32LE,
				space_id, &(hdr->sample_rate)) < 0)
		return -1;

	/* Block size */
	if (write_hdf_attr(dset_id, "block-size", H5T_STD_U32LE,
				space_id, &(hdr->block_size)) < 0)
		return -1;

	/* Gain */
	if (write_hdf_attr(dset_id, "gain", H5T_IEEE_F32LE,
				space_id, &(hdr->gain)) < 0)
		return -1;

	/* Offset */
	if (write_hdf_attr(dset_id, "offset", H5T_IEEE_F32LE,
				space_id, &(hdr->offset)) < 0)
		return -1;

	/* Date string */
	if (write_hdf_string_attr(dset_id, "date", hdr->date, space_id) < 0)
		return -1;

	/* Time string */
	if (write_hdf_string_attr(dset_id, "time", hdr->time, space_id) < 0)
		return -1;

	/* Room string */
	return write_hdf_string_attr(dset_id, "room", hdr->room, space_id);
}

int write_recording_attrs(hid_t file_id, bin_hdr_t *hdr) {
	herr_t status;
	hid_t attr_id;
	hsize_t dims[1] = {1};
	hid_t space_id = H5Screate_simple(1, dims, NULL);
	
	/* "Live" status. This means data is currently being recorded */
	char live = 0;
	attr_id = H5Acreate(file_id, "is-live", 
			H5T_STD_U8LE, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_STD_U8LE, &live);
	status = H5Aclose(attr_id);
	if (status)
		return status;

	/* Latest valid time index. This is used by applications reading files
	 * as they are being written.
	 */
	uint64_t idx = hdr->nsamples;
	attr_id = H5Acreate(file_id, "last-valid-sample",
			H5T_STD_U64LE, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_STD_U64LE, &idx);
	status = H5Aclose(attr_id);
	return status;
}

int write_data_to_hdf(hid_t dset_id, hid_t dspace_id, 
		int num_files, bin_hdr_t **hdr_list, FILE **fp_list) {

	/* Create dataspace information for blocks from the bin file, and select
	 * the appropriate hyperslab (all of the data)
	 */
	hsize_t block_dims[RANK] = {hdr_list[0]->nchannels, 
			hdr_list[0]->block_size};
	hid_t block_id = H5Screate_simple(RANK, block_dims, NULL);
	hsize_t block_start[RANK] = {0, 0};
	hsize_t block_count[RANK] = {hdr_list[0]->nchannels, 
			hdr_list[0]->block_size};
	herr_t status = H5Sselect_hyperslab(block_id, H5S_SELECT_SET, 
			block_start, NULL, block_count, NULL);

	/* Create parameters for the hyperslab selection of the data set */
	hsize_t start[RANK] = {0, 0};
	hsize_t count[RANK] = {hdr_list[0]->nchannels, 
			hdr_list[0]->block_size};

	int *block_offsets = calloc(num_files, sizeof(int));
	int *nblocks = calloc(num_files, sizeof(int));
	block_offsets[0] = 0;
	for (int fi = 0; fi < num_files; fi++) {
		nblocks[fi] = hdr_list[fi]->nsamples / hdr_list[fi]->block_size;
		if (fi >= 1)
			block_offsets[fi] = nblocks[fi - 1];
	}

	/* Write the data by file and block */
	for (int fi = 0; fi < num_files; fi++) {

		for (int bi = 0; bi < nblocks[fi]; bi++) {

			/* Read data from the bin file */
			int16_t *data = read_data_block(hdr_list[fi], fp_list[fi], bi);

			/* Offset the start of the H5 dataset and select the target hyperslab */
			start[1] = (fi * block_offsets[fi] * hdr_list[fi]->block_size + 
					bi * hdr_list[fi]->block_size);
			status = H5Sselect_hyperslab(dspace_id, H5S_SELECT_SET,
					start, NULL, count, NULL);
			if (status) {
				free(data);
				return status;
			}

			/* Write the data */
			status = H5Dwrite(dset_id, H5T_STD_I16LE, block_id, dspace_id, 
					H5P_DEFAULT, data);
			if (status) {
				free(data);
				return status;
			}
			free(data);
		}
	}
	return 0;
}

unsigned int *compute_num_bin_files(hid_t dset_id, unsigned int file_size, 
		unsigned int *num_files) {
	/* Get the size of the dataset */
	hid_t dspace_id = H5Dget_space(dset_id);
	hsize_t dims[RANK] = {0, 0};
	H5Sget_simple_extent_dims(dspace_id, dims, NULL);

	/* Get sample rate and determine necessary number of files */
	float fs = 0;
	hid_t attr = H5Aopen(dset_id, "sample-rate", H5P_DEFAULT);
	H5Aread(attr, H5T_IEEE_F32LE, &fs);
	unsigned int samples_per_full_file = file_size * fs;
	*num_files = ceil( ( (float) dims[1] ) / samples_per_full_file);

	/* Compute the number of samples in each file */
	unsigned int *ret = calloc(*num_files, sizeof(unsigned int));
	for (unsigned int i = 0; i < *num_files; i++)
		ret[i] = samples_per_full_file;
	if ( (samples_per_full_file * (*num_files)) > dims[1] )
		ret[(*num_files) - 1] = dims[1] - (samples_per_full_file * 
				(*num_files - 1));
	return ret;
}

int read_hdf_attr(hid_t dset_id, char *name, hid_t dtype, void *buf) {
	hid_t attr_id = H5Aopen(dset_id, name, H5P_DEFAULT);
	if (attr_id < 0)
		return -1;
	if (H5Aread(attr_id, dtype, buf) < 0) {
		H5Aclose(attr_id);
		return -1;
	}
	return H5Aclose(attr_id);
}

int read_hdf_string_attr(hid_t dset_id, char *name, uint32_t *sz, char **str) {
	hid_t string_type = H5Tcopy(H5T_C_S1);
	hid_t attr_id = H5Aopen(dset_id, name, H5P_DEFAULT);
	if (attr_id < 0)
		return -1;
	hsize_t size = H5Aget_storage_size(attr_id);
	H5Tset_size(string_type, size);
	*sz = size;
	*str = calloc(size, 1);
	if (!str) {
		H5Aclose(attr_id);
		return -1;
	}
	if (H5Aread(attr_id, string_type, *str) < 0) {
		H5Aclose(attr_id);
		return -1;
	}
	return H5Aclose(attr_id);
}

bin_hdr_t **create_bin_headers(hid_t dset_id, unsigned int num_files, 
		unsigned int *file_sizes, bool strict) {
	bin_hdr_t **hdr_list = calloc(num_files, sizeof(bin_hdr_t *));
	if (hdr_list == NULL)
		return NULL;
	for (unsigned int i = 0; i < num_files; i++) {
		hdr_list[i] = calloc(1, sizeof(bin_hdr_t));
		if (hdr_list[i] == NULL) {
			for (unsigned int j = 0; j < i; j++)
				free(hdr_list[j]);
			free(hdr_list);
			return NULL;
		}
	}

	for (unsigned int i = 0; i < num_files; i++) {

		/* Reference to this header, write size directly */
		bin_hdr_t *hdr = hdr_list[i];
		hdr->nsamples = file_sizes[i];

		/* Type */
		if ( (strict) || H5Aexists(dset_id, "bin-file-type") ) {
			if (read_hdf_attr(dset_id, "bin-file-type", 
						H5T_STD_I16LE, &(hdr->type)) < 0) {
				free_bin_headers(num_files, hdr_list);
				return NULL;
			}
		} else {
			hdr->type = DEFAULT_FILE_TYPE;
		}

		/* Version */
		if ( (strict) || H5Aexists(dset_id, "bin-file-version") ) {
			if (read_hdf_attr(dset_id, "bin-file-version", 
						H5T_STD_I16LE, &(hdr->version)) < 0) {
				free_bin_headers(num_files, hdr_list);
				return NULL;
			}
		} else {
			hdr->version = DEFAULT_FILE_VERSION;
		}

		/* Number of samples and channels */
		hsize_t dims[RANK] = {0, 0};
		hid_t dspace_id = H5Dget_space(dset_id);
		if (dspace_id < 0) {
			free_bin_headers(num_files, hdr_list);
			return NULL;
		}
		if (H5Sget_simple_extent_dims(dspace_id, dims, NULL) < 0) {
			H5Dclose(dspace_id);
			free_bin_headers(num_files, hdr_list);
			return NULL;
		}
		hdr->nchannels = dims[0];
		H5Sclose(dspace_id);

		/* Channel indices */
		hdr->channels = calloc(hdr->nchannels, sizeof(int16_t));
		if (hdr->channels == NULL) {
			free_bin_headers(num_files, hdr_list);
			return NULL;
		}
		for (uint32_t i = 0; i < hdr->nchannels; i++)
			hdr->channels[i] = i;

		/* Sample rate */
		if (read_hdf_attr(dset_id, "sample-rate", 
					H5T_IEEE_F32LE, &(hdr->sample_rate)) < 0) {
			free_bin_headers(num_files, hdr_list);
			return NULL;
		}

		/* Block size */
		if (read_hdf_attr(dset_id, "block-size", 
					H5T_STD_U32LE, &(hdr->block_size)) < 0) {
			free_bin_headers(num_files, hdr_list);
			return NULL;
		}

		/* Gain */
		if (read_hdf_attr(dset_id, "gain", 
					H5T_IEEE_F32LE, &(hdr->gain)) < 0) {
			free_bin_headers(num_files, hdr_list);
			return NULL;
		}

		/* Offset */
		if ( (strict) || H5Aexists(dset_id, "offset") ) {
			if (read_hdf_attr(dset_id, "offset", 
						H5T_IEEE_F32LE, &(hdr->offset)) < 0) {
				free_bin_headers(num_files, hdr_list);
				return NULL;
			}
		} else {
			hdr->offset = DEFAULT_OFFSET;
		}

		/* Date string */
		if ( (strict) || H5Aexists(dset_id, "date") ) {
			if (read_hdf_string_attr(dset_id, "date", 
						&(hdr->date_size), &(hdr->date)) < 0) {
				free_bin_headers(num_files, hdr_list);
				return NULL;
			}
		} else {
			hdr->date_size = DEFAULT_DATE_SIZE;
			hdr->date = DEFAULT_DATE;
		}

		/* Time string */
		if ( (strict) || H5Aexists(dset_id, "time") ) {
			if (read_hdf_string_attr(dset_id, "time", 
						&(hdr->time_size), &(hdr->time)) < 0) {
				free_bin_headers(num_files, hdr_list);
				return NULL;
			}
		} else {
			hdr->time_size = DEFAULT_TIME_SIZE;
			hdr->time = DEFAULT_TIME;
		}

		/* Room string */
		if ( (strict) || H5Aexists(dset_id, "room") ) {
			if (read_hdf_string_attr(dset_id, "room", 
						&(hdr->room_size), &(hdr->room)) < 0) {
				free_bin_headers(num_files, hdr_list);
				return NULL;
			}
		} else {
			hdr->room_size = DEFAULT_ROOM_SIZE;
			hdr->room = DEFAULT_ROOM;
		}

		/* Finally, compute total header size in bytes */
		hdr->header_size = (
				7 * sizeof(uint32_t) + 
				(2 + hdr->nchannels) * sizeof(int16_t) + 
				3 * sizeof(float) +
				hdr->date_size +
				hdr->time_size +
				hdr->room_size
			);
	}
	return hdr_list;
}

int write_bin_headers(unsigned int num_files, bin_hdr_t **hdr_list, 
		FILE **fp_list) {
	int ret = 0;
	for (unsigned int i = 0; i < num_files; i++) {
		if ((ret = write_bin_header(hdr_list[i], fp_list[i])) != 0)
			return -1;
	}
	return 0;
}

int write_data_to_bin(hid_t dset_id, hid_t dspace_id, 
		unsigned int num_files, bin_hdr_t **hdr_list, FILE **fp_list) {

	/* Create dataspace information for blocks from each bin file, and select
	 * the appropriate hyperslab (all of the data)
	 */
	hsize_t block_dims[RANK] = {hdr_list[0]->nchannels, 
			hdr_list[0]->block_size};
	hid_t block_id = H5Screate_simple(RANK, block_dims, NULL);
	hsize_t block_start[RANK] = {0, 0};
	hsize_t block_count[RANK] = {hdr_list[0]->nchannels, 
			hdr_list[0]->block_size};
	herr_t status = H5Sselect_hyperslab(block_id, H5S_SELECT_SET, 
			block_start, NULL, block_count, NULL);

	/* Create parameters for the hyperslab selection of the data set */
	hsize_t start[RANK] = {0, 0};
	hsize_t count[RANK] = {hdr_list[0]->nchannels, 
			hdr_list[0]->block_size};

	/* Allocate buffer to hold data from HDF5 file */
	int write_size = hdr_list[0]->nchannels * hdr_list[0]->block_size;
	int16_t *data_buffer = calloc(write_size, sizeof(int16_t));
	if (data_buffer == NULL) {
		H5Sclose(block_id);
		return -1;
	}

	/* Loop over files */
	unsigned int file_offset = 0;
	for (unsigned int fi = 0; fi < num_files; fi++) {

		/* Reference to this header and file */
		bin_hdr_t *hdr = hdr_list[fi];
		FILE *fp = fp_list[fi];

		/* Resize the file to hold all data, and seek to the end of the header */
		size_t total_size = (hdr->header_size + 
				hdr->nsamples * hdr->nchannels * sizeof(int16_t));
		int ret = ftruncate(fileno(fp), total_size);
		if (ret < 0) {
			H5Sclose(block_id);
			free(data_buffer);
			return -1;
		}
		ret = fseek(fp, hdr->header_size, SEEK_SET);
		if (ret < 0) {
			perror("fseek error");
			H5Sclose(block_id);
			free(data_buffer);
			return -1;
		}

		/* Write the data by blocks */
		int nblocks = hdr->nsamples / hdr->block_size;
		int nwritten = 0;
		bool swap = is_little_endian();
		for (int i = 0; i < nblocks; i++) {

			/* Offset the start of the H5 dataset and select the target hyperslab */
			start[1] = file_offset + i * hdr->block_size;
			status = H5Sselect_hyperslab(dspace_id, H5S_SELECT_SET,
					start, NULL, count, NULL);
			if (status) {
				free(data_buffer);
				H5Sclose(block_id);
				return status;
			}

			/* Read the data from the file */
			status = H5Dread(dset_id, H5T_STD_I16LE, block_id, dspace_id, 
					H5P_DEFAULT, data_buffer);
			if (status) {
				free(data_buffer);
				H5Sclose(block_id);
				return status;
			}

			/* Write to the file, swapping if needed */
			if (swap) {
				for (int i = 0; i < write_size; i++)
					data_buffer[i] = __builtin_bswap16(data_buffer[i]);
			}
			nwritten = fwrite(data_buffer, sizeof(int16_t), write_size, fp);
			if (nwritten != write_size) {
				perror("write error");
				H5Sclose(block_id);
				free(data_buffer);
				return -1;
			}
		}
		file_offset += hdr->nsamples;
	}

	/* Clean up */
	free(data_buffer);
	H5Sclose(block_id);

	return 0;
}

FILE **open_outfiles(char *basename, unsigned int num_files) {
	FILE **fp_list = calloc(num_files, sizeof(FILE *));
	if (!(fp_list))
		return fp_list;
	char *name;
	for (unsigned int i = 0; i < num_files; i++) {
		name = create_outfile_name(basename, i);
		if ( (fp_list[i] = fopen(name, "w+")) == NULL )
			return NULL;
	}
	return fp_list;
}

char *create_outfile_name(char *basename, unsigned int idx) {
	int sz = strlen(basename);
	int length = sz + strlen(BIN_FILE_EXTENSION) + 2; // 1 for idx, 1 for \0
	char *name = calloc(length, sizeof(char));
	if (!name)
		return name;
	memcpy(name, basename, sz);
	*(name + sz) = 'a' + idx;
	memcpy(name + sz + 1, BIN_FILE_EXTENSION, strlen(BIN_FILE_EXTENSION));
	return name;
}

