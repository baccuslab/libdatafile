/* hdftools.c
 * Implementation of library functions for converting between bin files
 * and HDF5 files
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <stdlib.h>
#include <unistd.h>

#include "hdf5.h"
#include "hdftools.h"
#include "bintools.h"

const int RANK = 2;

/* Helper function prototypes */
int write_dataset_attrs(hid_t dset_id, bin_hdr_t *hdr);
int write_recording_attrs(hid_t file_id, bin_hdr_t *hdr);
int write_data_to_hdf(hid_t dataset_id, hid_t dataspace_id, bin_hdr_t *hdr, FILE *fp);
int write_data_to_bin(hid_t dataset_id, hid_t dataspace_id, bin_hdr_t *hdr, FILE *fp);
bin_hdr_t *create_bin_header(hid_t dataset_id);

int bin2hdf(char *infile, char *outfile) {

	/* Open the source bin file and read its header */
	FILE *fp = fopen(infile, "r");
	if (fp == NULL) {
		perror("Could not open input bin file:");
		free(outfile);
		exit(EXIT_FAILURE);
	}
	bin_hdr_t *hdr = read_bin_header(fp);

	/* Identifiers and attributes for the HDF5 output file */
	hid_t file_id, dset_id, dspace_id, dset_create_props;
	hsize_t dims[RANK] = {hdr->nchannels, hdr->nsamples};

	/* Open the output file. No compression for now */
	if ((file_id = H5Fcreate(outfile, H5F_ACC_TRUNC, 
			H5P_DEFAULT, H5P_DEFAULT)) < 0) {
		printf("Could not open output file\n");
		free_bin_header(hdr);
		fclose(fp);
		return -1;
	}

	/* Create property list giving chunked data */
	if ((dset_create_props = H5Pcreate(H5P_DATASET_CREATE)) < 0) {
		printf("Could not create dataset properties\n");
		free_bin_header(hdr);
		fclose(fp);
		H5Fclose(file_id);
		return -1;
	}
	hsize_t chunk_size[RANK] = {hdr->nchannels, hdr->block_size};
	if ((H5Pset_chunk(dset_create_props, RANK, chunk_size)) < 0) {
		printf("Could not set dataset properties\n");
		free_bin_header(hdr);
		fclose(fp);
		H5Fclose(file_id);
		return -1;
	}
	
	/* Create actual dataspace and dataset */
	if ((dspace_id = H5Screate_simple(RANK, dims, NULL)) < 0) {
		printf("Could not create dataspace\n");
		free_bin_header(hdr);
		fclose(fp);
		H5Fclose(file_id);
		H5Pclose(dset_create_props);
		return -1;
	}
	if ((dset_id = H5Dcreate2(file_id, "data", 
			H5T_STD_I16LE, dspace_id, 
			H5P_DEFAULT, dset_create_props, H5P_DEFAULT)) < 0) {
		printf("COuld not create dataset\n");
		free_bin_header(hdr);
		fclose(fp);
		H5Fclose(file_id);
		H5Pclose(dset_create_props);
		return -1;
	}

	/* Write the information in the header into the attributes of the dataset */
	if (write_dataset_attrs(dset_id, hdr)) {
		printf("Could not write dataset attributes\n");
		H5Sclose(dspace_id);
		H5Dclose(dset_id);
		H5Fclose(file_id);
		fclose(fp);
		return -1;
	}

	/* Write recording status metadata as attributes of the file */
	if (write_recording_attrs(file_id, hdr)) {
		printf("Could not write recording attributes\n");
		H5Sclose(dspace_id);
		H5Dclose(dset_id);
		H5Fclose(file_id);
		fclose(fp);
		return -1;
	}

	/* Write the data itself */
	if (write_data_to_hdf(dset_id, dspace_id, hdr, fp)) {
		printf("Could not write data\n");
		H5Sclose(dspace_id);
		H5Dclose(dset_id);
		H5Fclose(file_id);
		fclose(fp);
		return -1;
	}

	/* Close up shop */
	if (H5Sclose(dspace_id)) {
		H5Dclose(dset_id);
		H5Fclose(file_id);
		fclose(fp);
		return -1;
	}
	if (H5Dclose(dset_id)) {
		H5Fclose(file_id);
		fclose(fp);
		return -1;
	}
	if (H5Fclose(file_id)) {
		fclose(fp);
		return -1;
	}
	if (fclose(fp))
		return -1;

	return 0;
}

int hdf2bin(char *infile, char *outfile) {

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

	/* Create a corresponding header for the bin file */
	bin_hdr_t *hdr = create_bin_header(dset_id);
	if (hdr == NULL) {
		printf("Could not create bin header\n");
		H5Fclose(file_id);
		H5Dclose(dset_id);
		H5Sclose(dspace_id);
		return -1;
	}

	/* Open the bin file and write the header */
	FILE *fp = fopen(outfile, "w+");
	if (fp == NULL) {
		printf("Could not open output file: %s\n", outfile);
		H5Fclose(file_id);
		H5Dclose(dset_id);
		H5Sclose(dspace_id);
		free_bin_header(hdr);
		return -1;
	}
	if (write_bin_header(hdr, fp)) {
		printf("Could not write bin header\n");
		H5Fclose(file_id);
		H5Dclose(dset_id);
		H5Sclose(dspace_id);
		free_bin_header(hdr);
		fclose(fp);
		return -1;
	}

	/* Write the HDF5 dataset to the file */
	if (write_data_to_bin(dset_id, dspace_id, hdr, fp)) {
		printf("Could not write bin data\n");
		H5Fclose(file_id);
		H5Dclose(dset_id);
		H5Sclose(dspace_id);
		free_bin_header(hdr);
		fclose(fp);
		return -1;
	}

	return 0;
}

int write_dataset_attrs(hid_t dset_id, bin_hdr_t *hdr) {
	herr_t status;
	hid_t attr_id;
	hsize_t dims[1] = {1};
	hid_t space_id = H5Screate_simple(1, dims, NULL);

	/* Type */
	attr_id = H5Acreate(dset_id, "bin-file-type",
			H5T_STD_I16LE, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_STD_I16LE, &(hdr->type));
	status = H5Aclose(attr_id);
	if (status)
		return status;

	/* Version */
	attr_id = H5Acreate(dset_id, "bin-file-version",
			H5T_STD_I16LE, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_STD_I16LE, &(hdr->version));
	status = H5Aclose(attr_id);
	if (status)
		return status;

	/* Sample rate */
	attr_id = H5Acreate(dset_id, "sample-rate", 
			H5T_IEEE_F32LE, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_IEEE_F32LE, &(hdr->sample_rate));
	status = H5Aclose(attr_id);
	if (status)
		return status;

	/* Block size */
	attr_id = H5Acreate(dset_id, "block-size",
			H5T_STD_U32LE, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_STD_U32LE, &(hdr->block_size));
	status = H5Aclose(attr_id);
	if (status)
		return status;

	/* Gain */
	attr_id = H5Acreate(dset_id, "gain",
			H5T_IEEE_F32LE, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_IEEE_F32LE, &(hdr->gain));
	status = H5Aclose(attr_id);
	if (status)
		return status;

	/* Offset */
	attr_id = H5Acreate(dset_id, "offset",
			H5T_IEEE_F32LE, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_IEEE_F32LE, &(hdr->offset));
	status = H5Aclose(attr_id);
	if (status)
		return status;

	/* Date string */
	hid_t string_type = H5Tcopy(H5T_C_S1);
	H5Tset_size(string_type, hdr->date_size);
	attr_id = H5Acreate(dset_id, "date",
			string_type, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, string_type, hdr->date);
	status = H5Aclose(attr_id);
	if (status)
		return status;

	/* Time string */
	H5Tset_size(string_type, hdr->time_size);
	attr_id = H5Acreate(dset_id, "time",
			string_type, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, string_type, hdr->time);
	status = H5Aclose(attr_id);
	if (status)
		return status;

	/* Room string */
	H5Tset_size(string_type, hdr->room_size);
	attr_id = H5Acreate(dset_id, "room",
			string_type, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, string_type, hdr->room);
	status = H5Aclose(attr_id);
	return status;
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
	attr_id = H5Acreate(file_id, "latest-idx",
			H5T_STD_U64LE, space_id, H5P_DEFAULT, H5P_DEFAULT);
	H5Awrite(attr_id, H5T_STD_U64LE, &idx);
	status = H5Aclose(attr_id);
	return status;
}

int write_data_to_hdf(hid_t dset_id, hid_t dspace_id, bin_hdr_t *hdr, FILE *fp) {

	/* Create dataspace information for blocks from the bin file, and select
	 * the appropriate hyperslab (all of the data)
	 */
	hsize_t block_dims[RANK] = {hdr->nchannels, hdr->block_size};
	hid_t block_id = H5Screate_simple(RANK, block_dims, NULL);
	hsize_t block_start[RANK] = {0, 0};
	hsize_t block_count[RANK] = {hdr->nchannels, hdr->block_size};
	herr_t status = H5Sselect_hyperslab(block_id, H5S_SELECT_SET, 
			block_start, NULL, block_count, NULL);

	/* Create parameters for the hyperslab selection of the data set */
	hsize_t start[RANK] = {0, 0};
	hsize_t count[RANK] = {hdr->nchannels, hdr->block_size};

	/* Write the data by blocks */
	int nblocks = hdr->nsamples / hdr->block_size;
	for (int i = 0; i < nblocks; i++) {

		/* Read data from the bin file */
		int16_t *data = read_data_block(hdr, fp, i);

		/* Offset the start of the H5 dataset and select the target hyperslab */
		start[1] = i * hdr->block_size;
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
	return 0;
}

bin_hdr_t *create_bin_header(hid_t dset_id) {
	bin_hdr_t *hdr = calloc(1, sizeof(bin_hdr_t));
	if (hdr == NULL)
		return NULL;
	hid_t attr_id;

	/* Type */
	if ((attr_id = H5Aopen(dset_id, "bin-file-type", H5P_DEFAULT)) < 0) {
		free(hdr);
		return NULL;
	}
	if (H5Aread(attr_id, H5T_STD_I16LE, &(hdr->type)) < 0) {
		free(hdr);
		H5Aclose(attr_id);
		return NULL;
	}
	if (H5Aclose(attr_id) < 0) {
		free(hdr);
		return NULL;
	}

	/* Version */
	if ((attr_id = H5Aopen(dset_id, "bin-file-version", H5P_DEFAULT)) < 0) {
		free(hdr);
		return NULL;
	}
	if (H5Aread(attr_id, H5T_STD_I16LE, &(hdr->version)) < 0) {
		free(hdr);
		H5Aclose(attr_id);
		return NULL;
	}
	if (H5Aclose(attr_id) < 0) {
		free(hdr);
		return NULL;
	}

	/* Number of samples and channels */
	hsize_t dims[RANK] = {0, 0};
	hid_t dspace_id = H5Dget_space(dset_id);
	if (dspace_id < 0) {
		free(hdr);
		return NULL;
	}
	if (H5Sget_simple_extent_dims(dspace_id, dims, NULL) < 0) {
		H5Dclose(dspace_id);
		free(hdr);
		return NULL;
	}
	hdr->nchannels = dims[0];
	hdr->nsamples = dims[1];
	H5Sclose(dspace_id);

	/* Channel indices */
	hdr->channels = calloc(hdr->nchannels, sizeof(int16_t));
	if (hdr->channels == NULL) {
		free(hdr);
		return NULL;
	}
	for (uint32_t i = 0; i < hdr->nchannels; i++)
		hdr->channels[i] = i;

	/* Sample rate */
	if ((attr_id = H5Aopen(dset_id, "sample-rate", H5P_DEFAULT)) < 0) {
		free(hdr);
		return NULL;
	}
	if (H5Aread(attr_id, H5T_IEEE_F32LE, &(hdr->sample_rate)) < 0) {
		free(hdr);
		H5Aclose(attr_id);
		return NULL;
	}
	if (H5Aclose(attr_id) < 0) {
		free(hdr);
		return NULL;
	}

	/* Block size */
	if ((attr_id = H5Aopen(dset_id, "block-size", H5P_DEFAULT)) < 0) {
		free(hdr);
		return NULL;
	}
	if (H5Aread(attr_id, H5T_STD_U32LE, &(hdr->block_size)) < 0) {
		free(hdr);
		H5Aclose(attr_id);
		return NULL;
	}
	if (H5Aclose(attr_id) < 0) {
		free(hdr);
		return NULL;
	}

	/* Gain */
	if ((attr_id = H5Aopen(dset_id, "gain", H5P_DEFAULT)) < 0) {
		free(hdr);
		return NULL;
	}
	if (H5Aread(attr_id, H5T_IEEE_F32LE, &(hdr->gain)) < 0) {
		free(hdr);
		return NULL;
	}
	if (H5Aclose(attr_id) < 0) {
		free(hdr);
		return NULL;
	}

	/* Offset */
	if ((attr_id = H5Aopen(dset_id, "offset", H5P_DEFAULT)) < 0) {
		free(hdr);
		return NULL;
	}
	if (H5Aread(attr_id, H5T_IEEE_F32LE, &(hdr->offset)) < 0) {
		free(hdr);
		return NULL;
	}
	if (H5Aclose(attr_id) < 0) {
		free(hdr);
		return NULL;
	}

	/* Date string */
	hid_t string_type = H5Tcopy(H5T_C_S1);
	if ((attr_id = H5Aopen(dset_id, "date", H5P_DEFAULT)) < 0) {
		free(hdr);
		return NULL;
	}
	hsize_t sz = H5Aget_storage_size(attr_id);
	H5Tset_size(string_type, sz);
	hdr->date_size = sz;
	hdr->date = calloc(sz + 1, 1);
	if (hdr->date == NULL) {
		free(hdr);
		H5Aclose(attr_id);
		return NULL;
	}
	if (H5Aread(attr_id, string_type, hdr->date) < 0) {
		free(hdr);
		free(hdr->date);
		H5Aclose(attr_id);
		return NULL;
	}
	if (H5Aclose(attr_id) < 0) {
		free(hdr);
		free(hdr->date);
		H5Aclose(attr_id);
		return NULL;
	}

	/* Time string */
	if ((attr_id = H5Aopen(dset_id, "time", H5P_DEFAULT)) < 0) {
		free(hdr);
		free(hdr->date);
		return NULL;
	}
	sz = H5Aget_storage_size(attr_id);
	H5Tset_size(string_type, sz);
	hdr->time_size = sz;
	hdr->time = calloc(sz + 1, 1);
	if (hdr->time == NULL) {
		free(hdr);
		free(hdr->date);
		H5Aclose(attr_id);
		return NULL;
	}
	if (H5Aread(attr_id, string_type, hdr->time) < 0) {
		free(hdr);
		free(hdr->date);
		free(hdr->time);
		H5Aclose(attr_id);
		return NULL;
	}
	if (H5Aclose(attr_id) < 0) {
		free(hdr);
		free(hdr->date);
		free(hdr->time);
		return NULL;
	}

	/* Room string */
	if ((attr_id = H5Aopen(dset_id, "room", H5P_DEFAULT)) < 0) {
		free(hdr);
		free(hdr->date);
		return NULL;
	}
	sz = H5Aget_storage_size(attr_id);
	H5Tset_size(string_type, sz);
	hdr->room_size = sz;
	hdr->room = calloc(sz + 1, 1);
	if (hdr->room == NULL) {
		free(hdr);
		free(hdr->date);
		H5Aclose(attr_id);
		return NULL;
	}
	if (H5Aread(attr_id, string_type, hdr->room) < 0) {
		free(hdr);
		free(hdr->date);
		free(hdr->time);
		free(hdr->room);
		H5Aclose(attr_id);
		return NULL;
	}
	if (H5Aclose(attr_id) < 0) {
		free(hdr);
		free(hdr->date);
		free(hdr->time);
		free(hdr->room);
		H5Aclose(attr_id);
		return NULL;
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

	return hdr;
}

int write_data_to_bin(hid_t dset_id, hid_t dspace_id, bin_hdr_t *hdr, FILE *fp) {

	/* Create dataspace information for blocks from the bin file, and select
	 * the appropriate hyperslab (all of the data)
	 */
	hsize_t block_dims[RANK] = {hdr->nchannels, hdr->block_size};
	hid_t block_id = H5Screate_simple(RANK, block_dims, NULL);
	hsize_t block_start[RANK] = {0, 0};
	hsize_t block_count[RANK] = {hdr->nchannels, hdr->block_size};
	herr_t status = H5Sselect_hyperslab(block_id, H5S_SELECT_SET, 
			block_start, NULL, block_count, NULL);

	/* Create parameters for the hyperslab selection of the data set */
	hsize_t start[RANK] = {0, 0};
	hsize_t count[RANK] = {hdr->nchannels, hdr->block_size};

	/* Allocate buffer to hold data from HDF5 file */
	int write_size = hdr->nchannels * hdr->block_size;
	int16_t *data_buffer = calloc(write_size, sizeof(int16_t));
	if (data_buffer == NULL) {
		H5Sclose(block_id);
		return -1;
	}

	/* Resize the file to hold all data, and seek to the end of the header */
	off_t total_size = (hdr->header_size + 
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
		start[1] = i * hdr->block_size;
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

	/* Clean up */
	free(data_buffer);
	H5Sclose(block_id);

	return 0;
}
