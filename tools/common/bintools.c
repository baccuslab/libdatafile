/* bintools.c
 * Implementation of functions and types for interacting with the *.bin
 * data files traditionally used in Baccus lab MEA recordings.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "bintools.h"

static inline float __bswapf(float x) {
	float ret = 0;
	char *rp = (char *) &ret;
	char *xp = (char *) &x;
	*rp = *(xp + 3);
	*(rp + 1) = *(xp + 2);
	*(rp + 2) = *(xp + 1);
	*(rp + 3) = *xp;
	return ret;
}

bool is_little_endian(void) {
	uint16_t i = 1;
	char *ptr = (char *) &i;
	return ((*ptr) == 0x01);
}

bin_hdr_t *read_bin_header(FILE *fp) {

	/* Determine current machine's endianness. Bin files are historically
	 * big-endian, and so bytes may need to be swapped.
	 */
	bool swap = is_little_endian();

	/* Rewind file and allocate memory */
	rewind(fp);
	bin_hdr_t *hdr;
	if ((hdr = calloc(1, sizeof(bin_hdr_t))) == NULL)
		return NULL;

	fread(&hdr->header_size, sizeof(uint32_t), 1, fp);
	if (swap)
		hdr->header_size = __builtin_bswap32(hdr->header_size);
	size_t remaining = hdr->header_size - sizeof(uint32_t);
	void *header_mem;
	if ((header_mem = calloc(1, remaining)) == NULL)
		return NULL;
	fread(header_mem, sizeof(char), remaining, fp);
	void *read_ptr = header_mem;

	/* Read type, version, etc */
	hdr->type = *(uint16_t *) read_ptr;
	if (swap)
		hdr->type = __builtin_bswap16(hdr->type);
	read_ptr = (int16_t *) read_ptr + 1;

	hdr->version = *(int16_t *) read_ptr;
	if (swap)
		hdr->version = __builtin_bswap16(hdr->version);
	read_ptr = (int16_t *) read_ptr + 1;

	hdr->nsamples = *(uint32_t *) read_ptr;
	if (swap)
		hdr->nsamples = __builtin_bswap32(hdr->nsamples);
	read_ptr = (uint32_t *) read_ptr + 1;

	hdr->nchannels = *(uint32_t *) read_ptr;
	if (swap)
		hdr->nchannels = __builtin_bswap32(hdr->nchannels);
	read_ptr = (uint32_t *) read_ptr + 1;

	/* Allocate memory and read channel indices */
	if ((hdr->channels = calloc(hdr->nchannels, sizeof(int16_t))) == NULL) {
		free(header_mem);
		return NULL;
	}
	for (uint32_t i = 0; i < hdr->nchannels; i++) {
		*(hdr->channels + i) = *((int16_t *) read_ptr + i);
		if (swap)
			*(hdr->channels + i) = __builtin_bswap16(*(hdr->channels + i));
	}
	read_ptr = (int16_t *) read_ptr + hdr->nchannels;

	/* Read the block and sample information */
	hdr->sample_rate = *(float *) read_ptr;
	if (swap)
		hdr->sample_rate = __bswapf(hdr->sample_rate);
	read_ptr = (float *) read_ptr + 1;

	hdr->block_size = *(uint32_t *) read_ptr;
	if (swap)
		hdr->block_size = __builtin_bswap32(hdr->block_size);
	read_ptr = (uint32_t *) read_ptr + 1;

	hdr->gain = *(float *) read_ptr;
	if (swap)
		hdr->gain = __bswapf(hdr->gain);
	read_ptr = (float *) read_ptr + 1;

	hdr->offset = *(float *) read_ptr;
	if (swap)
		hdr->offset = __bswapf(hdr->offset);
	read_ptr = (float *) read_ptr + 1;

	/* Read string arguments. 
	 * Note that they require no byte-swapping, but they are NOT
	 * '\0'-terminated so we calloc 1 extra byte so that we can 
	 * call strlen to get the same value returned by the header itself.
	 */
	hdr->date_size = *(uint32_t *) read_ptr;
	if (swap)
		hdr->date_size = __builtin_bswap32(hdr->date_size);
	read_ptr = (uint32_t *) read_ptr + 1;
	if ((hdr->date = calloc(hdr->date_size + 1, sizeof(char))) == NULL) {
		free(header_mem);
		return NULL;
	}
	memcpy(hdr->date, read_ptr, hdr->date_size);
	read_ptr = (char *) read_ptr + hdr->date_size;

	hdr->time_size = *(uint32_t *) read_ptr;
	if (swap)
		hdr->time_size = __builtin_bswap32(hdr->time_size);
	read_ptr = (uint32_t *) read_ptr + 1;
	if ((hdr->time = calloc(hdr->time_size + 1, sizeof(char))) == NULL) {
		free(header_mem);
		return NULL;
	}
	memcpy(hdr->time, read_ptr, hdr->time_size);
	read_ptr = (char *) read_ptr + hdr->time_size;

	hdr->room_size = *(uint32_t *) read_ptr;
	if (swap)
		hdr->room_size = __builtin_bswap32(hdr->room_size);
	read_ptr = (uint32_t *) read_ptr + 1;
	if ((hdr->room = calloc(hdr->room_size + 1, sizeof(char))) == NULL) {
		free(header_mem);
		return NULL;
	}
	memcpy(hdr->room, read_ptr, hdr->room_size);

	/* Clean up and return */
	free(header_mem);
	return hdr;
}

int write_bin_header(bin_hdr_t *hdr, FILE *fp) {
	/* Allocate a buffer for the raw data */
	bool swap = is_little_endian();
	char *buffer = malloc(hdr->header_size);
	if (buffer == NULL)
		return -1;
	char *ptr = buffer;

	*(uint32_t *) ptr = swap ? __builtin_bswap32(hdr->header_size) : hdr->header_size;
	ptr += sizeof(uint32_t);
	*(int16_t *) ptr = swap ? __builtin_bswap16(hdr->type) : hdr->type;
	ptr += sizeof(int16_t);
	*(int16_t *) ptr = swap ? __builtin_bswap16(hdr->version) : hdr->version;
	ptr += sizeof(int16_t);
	*(uint32_t *) ptr = swap ? __builtin_bswap32(hdr->nsamples) : hdr->nsamples;
	ptr += sizeof(uint32_t);
	*(uint32_t *) ptr = swap ? __builtin_bswap32(hdr->nchannels) : hdr->nchannels;
	ptr += sizeof(uint32_t);
	memcpy(ptr, hdr->channels, sizeof(int16_t) * hdr->nchannels);
	if (swap) {
		for (uint32_t i = 0; i < hdr->nchannels; i++)
			*((int16_t *) ptr + i) = __builtin_bswap16(*(int16_t *) ptr + i);
	}
	ptr += sizeof(int16_t) * hdr->nchannels;
	*(float *) ptr = swap ? __bswapf(hdr->sample_rate) : hdr->sample_rate;
	ptr += sizeof(float);
	*(uint32_t *) ptr = swap ? __builtin_bswap32(hdr->block_size) : hdr->block_size;
	ptr += sizeof(uint32_t);
	*(float *) ptr = swap ? __bswapf(hdr->gain) : hdr->gain;
	ptr += sizeof(float);
	*(float *) ptr = swap ? __bswapf(hdr->offset) : hdr->offset;
	ptr += sizeof(float);
	*(uint32_t *) ptr = swap ? __builtin_bswap32(hdr->date_size) : hdr->date_size;
	ptr += sizeof(uint32_t);
	memcpy(ptr, hdr->date, hdr->date_size);
	ptr += hdr->date_size;
	*(uint32_t *) ptr = swap ? __builtin_bswap32(hdr->time_size) : hdr->time_size;
	ptr += sizeof(uint32_t);
	memcpy(ptr, hdr->time, hdr->time_size);
	ptr += hdr->time_size;
	*(uint32_t *) ptr = swap ? __builtin_bswap32(hdr->room_size) : hdr->room_size;
	ptr += sizeof(uint32_t);
	memcpy(ptr, hdr->room, hdr->room_size);
	ptr += hdr->room_size;

	/* Write out to the file */
	clearerr(fp);
	rewind(fp);
	if (ferror(fp)) {
		free(buffer);
		return -1;
	}
	size_t nwritten = fwrite(buffer, 1, hdr->header_size, fp);
	if (nwritten < hdr->header_size) {
		free(buffer);
		return -1;
	}
	free(buffer);
	return 0;
}

int16_t *read_data_block(bin_hdr_t *hdr, FILE *fp, int block_num) {
	/* Seek to location in the file */
	unsigned long offset = (hdr->header_size + 
			hdr->nchannels * hdr->block_size * sizeof(int16_t) * block_num);
	if (fseek(fp, offset, SEEK_SET) != 0)
		return NULL;

	/* Allocate space for data */
	int16_t *data = calloc(hdr->nchannels * hdr->block_size, sizeof(int16_t));
	if (data == NULL)
		return NULL;

	/* Read data, swap if necessary */
	size_t nread = fread((void *) data, sizeof(int16_t), 
			hdr->block_size * hdr->nchannels, fp);
	if ((nread == 0) || (nread != hdr->block_size * hdr->nchannels)) {
		free(data);
		return NULL;
	}
	if (is_little_endian()) {
		for (uint32_t i = 0; i < hdr->nchannels; i++) {
			for (uint32_t j = 0; j < hdr->block_size; j++) {
				*(data + i * hdr->block_size + j) = __builtin_bswap16(
						*(data + i * hdr->block_size + j));
			}
		}
	}
	return data;
}

void free_bin_header(bin_hdr_t *hdr) {
	free(hdr->date);
	free(hdr->time);
	free(hdr->room);
	free(hdr->channels);
	free(hdr);
}

