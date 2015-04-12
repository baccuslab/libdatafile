/* bintools.h
 * Header file describing public types and fuctions used in reading
 * the *.bin files traditionally used to store experimental data.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _BINTOOLS_H_
#define _BINTOOLS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct {
	uint32_t header_size;	// Total header size in bytes
	int16_t type;			// Type ID
	int16_t version;		// File version ID
	uint32_t nsamples;		// Total, per channel
	uint32_t nchannels;		// Total number of channels
	int16_t *channels;		// Channel indices
	float sample_rate;		// Samples per second
	uint32_t block_size;	// Size of data block (single channel)
	float gain;				// Amplifier gain
	float offset;			// Amplifier offset
	uint32_t date_size;		// Size of date string
	char *date;				// Date string
	uint32_t time_size;		// Size of time string
	char *time;				// Time string
	uint32_t room_size;	 	// Size of room string
	char *room;				// Room string
} bin_hdr_t;

bin_hdr_t *read_bin_header(FILE *fp);
void free_bin_header(bin_hdr_t *hdr);
int write_bin_header(bin_hdr_t *hdr, FILE *fp);
int16_t *read_data_block(bin_hdr_t *hdr, FILE *fp, int block_num);

/* This helper function returns true if the current machine is
 * little-endian. This useful feature is exported here because 
 * bin-files are traditionally in big-endian byte order, information
 * which is useful in converting between HDF5 and bin files, for example
 */
bool is_little_endian(void); 

#endif

