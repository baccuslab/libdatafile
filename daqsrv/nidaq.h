/* nidaq.h
 * Wrappers and types used to interface directly with the NIDAQmxBase 
 * driver library.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _NIDAQ_H_
#define _NIDAQ_H_

#include <stdint.h>

#ifdef _WIN32
#include "NIDAQmx.h"
#define NIDAQ_TRIGGER_EDGE DAQmx_Val_Rising
#define NIDAQ_SAMPLE_MODE DAQmx_Val_ContSamps
#define NIDAQ_TIMEOUT 10
#define NIDAQ_FILL_MODE DAQmx_Val_GroupByChannel
#define NIDAQ_TERMINAL_CONFIG DAQmx_Val_NRSE
#define NIDAQ_TIMING_SOURCE "OnboardClock"
#endif

#define NIDAQ_ERR_BUFFER_SIZE 2048
#define NIDAQ_NUM_CHANNELS 64
#define NIDAQ_BLOCK_SIZE 1000
#define NIDAQ_SAMPLE_RATE 10000
#define NIDAQ_DATE_FORMAT "%Y-%m-%d_%H-%M-%S"
#define NIDAQ_DATE_BUFFER_SIZE 64

#ifdef _WIN32
/* The task_t type wraps the information important to 
 * run an actual NIDAQmxBase task.
 */
typedef struct {
	TaskHandle handle;
	uint16_t nchannels; 	// Number of channels in experiment
	float length;		// Length of experiment in seconds
	uint64_t nsamples;	// Number of samples in experiment
	uint32_t block_size;	// Size of a data message block
	float sample_rate;	// Sample rate of A/D
	float adc_range;	// Voltage range of ADC
	float adc_resolution;	// Voltage resolution
	uint32_t trigger_length;// Length of trigger string
	char *trigger;		// Triggering method
	uint32_t date_length;	// Length of date string
	char *date;		// Time/date of experiment
} task_t;

task_t *init_task(char *trigger, uint32_t block_size, 
	float length, float adc_range);
int16_t *get_next_data_block(task_t *task);
void free_task(task_t *task);

#endif
#endif // include guard

