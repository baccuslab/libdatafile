/* messages.h
 * This file describes the messages used in the NI-DAQ server application.
 * 
 * There are just a few messages defined. These just implement the "business 
 * logic" of an experiment, rather than providing a true RPC mechanism. This
 * is becuase there are very few reasonable options that can be specified
 * in terms of configuration of the NI-DAQ device itself. Thus the client
 * really can just tell the server to "intialise an experiment", rather than
 * deal with the details of configuration.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include <stdint.h>
#include <stdbool.h>
#include "nidaq.h"

typedef enum {
	INIT_EXPERIMENT = 0x00, // Request to initialise experiment
	EXPT_PARAMS_REQ = 0x01,	// Request to send back experiment parameters
	EXPT_PARAMS = 0x02,	// Actual experiment parameters
	CHECK_READY = 0x03,	// Check if experiment is ready to begin
	READY_STATUS = 0x04,	// 0 if not ready, 1 if ready
	START_EXPT = 0x05,	// Start collecting samples
	DATA_CHUNK = 0x06,	// A data chunk message
	CLOSE = 0x07,		// Request that server stop the task and cleanup
	ERROR_MSG = 0x08	// Error message from server to client
} msg_t;

typedef struct {
	msg_t msg_type;
	uint32_t msg_size;	// Total size of message, bytes
	uint32_t trigger_length;// Length of trigger string
	char *trigger;		// How to trigger experiment start
	float expt_length;	// Length of experiment in seconds
	float adc_range;	// Voltage range of ADC
	uint32_t block_size;	// Block size, in samples, that will be sent
				// as a single data message over the wire.
} init_msg_t;

typedef struct {
	msg_t msg_type;
	uint32_t msg_size;	// Total size of message, bytes
	float nseconds;		// Length of epxeriment in seconds
	uint32_t nsamples;	// Length of experiment in samples
	float sample_rate;	// Sample rate of NI-DAQ A/D
	float adc_range;	// Voltage range of ADC
	float adc_resolution;// Resolution of ADC
	uint32_t block_size;	// Size of a data block message
	uint16_t nchannels;	// Number of channels
	uint32_t trigger_length;// Length of trigger string
	char *trigger;		// Triggering method
	uint32_t date_length;	// Length of date string
	char *date;		// Date string
} params_msg_t;

typedef struct {
	msg_t msg_type;
	uint32_t msg_size;	// Total size of message, bytes
	uint16_t nchannels;
	uint32_t nsamples;
	int16_t *data;
} data_msg_t;

typedef struct {
	msg_t msg_type;
	uint32_t msg_size;	// Total size of message, bytes
	uint32_t err_msg_size;
	char *err_msg;
} err_msg_t;


#ifdef _WIN32_WINNT
int read_msg_header(char *data, msg_t *type, uint32_t *size);
init_msg_t *parse_init_msg(char *data);
void free_init_msg(init_msg_t *msg);
params_msg_t *create_params_msg(task_t *task);
void free_params_msg(params_msg_t *msg);
data_msg_t *create_data_msg(task_t *task, int16_t *data);
void free_data_msg(data_msg_t *msg);
err_msg_t *create_err_msg(uint32_t sz, char *msg);
#endif

#endif // include guard

