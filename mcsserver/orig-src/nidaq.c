/* nidaq.c
 * Implementation of the actual interface with the NIDAQmxBase library.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */


#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "nidaq.h"

int get_task_time(task_t *task) {
	time_t t;
	struct tm *tmp;
	t = time(NULL);
	tmp = localtime(&t);
	if (tmp == NULL)
		return -1;
	char buffer[NIDAQ_DATE_BUFFER_SIZE] = {'\0'};
	strftime(buffer, NIDAQ_DATE_BUFFER_SIZE, NIDAQ_DATE_FORMAT, tmp);
	task->date_length = strlen(buffer);
	task->date = calloc(task->date_length + 1, 1);
	if (task->date == NULL)
		return -1;
	strncpy(task->date, buffer, task->date_length);
	return 0;
}

task_t *init_task(char *trigger, uint32_t block_size, 
		float length, float adc_range) {
	task_t *task = calloc(1, sizeof(task_t));
	if (task == NULL)
		return NULL;
	task->running = false;
	task->finished = false;

	task->trigger_length = strlen(trigger);
	task->trigger = strdup(trigger);
	task->length = length;
	task->block_size = block_size;
	task->adc_range = adc_range;
	task->adc_resolution = (adc_range * 2) / (1 << 16);

	/* Get date */
	if (get_task_time(task) != 0) {
		free(task);
		return NULL;
	}

	/* Compute number of total samples */
	task->nsamples = NIDAQ_SAMPLE_RATE * length;
	
	/* Setup task */
	int32 status = 0;
	status = DAQmxCreateTask("", &(task->handle));
	if (status != 0) {
		char buf[NIDAQ_ERR_BUFFER_SIZE];
		DAQmxGetExtendedErrorInfo(buf, NIDAQ_ERR_BUFFER_SIZE);
		printf("error: %s\n", buf);
		free_task(task);
		return NULL;
	}

	/* Add channels to task */
	task->nchannels = NIDAQ_NUM_CHANNELS;
	task->sample_rate = NIDAQ_SAMPLE_RATE;
	status = DAQmxCreateAIVoltageChan(task->handle, "Dev1/ai0:63",
		NULL, NIDAQ_TERMINAL_CONFIG, -task->adc_range, task->adc_range,
		DAQmx_Val_Volts, NULL);
	if (status < 0) {
		char buf[NIDAQ_ERR_BUFFER_SIZE];
		DAQmxGetExtendedErrorInfo(buf, NIDAQ_ERR_BUFFER_SIZE);
		printf("error: %s\n", buf);
		free_task(task);
		return NULL;
	}

	/* Initialize timing */
	status = DAQmxCfgSampClkTiming(task->handle,
		NIDAQ_TIMING_SOURCE, NIDAQ_SAMPLE_RATE, NIDAQ_TRIGGER_EDGE, NIDAQ_SAMPLE_MODE,
		block_size);
	if (status < 0) {
		char buf[NIDAQ_ERR_BUFFER_SIZE];
		DAQmxGetExtendedErrorInfo(buf, NIDAQ_ERR_BUFFER_SIZE);
		printf("error: %s\n", buf);
		free_task(task);
		return NULL;
	}

	/* Initialize triggering */
	if (strcmp(trigger, "photodiode") == 0) {
		float TRIGGER_LEVEL = (adc_range / 4); // Halfway to positive max
		status = DAQmxCfgAnlgEdgeStartTrig(task->handle, 
			"Dev1/ai0", NIDAQ_TRIGGER_EDGE, TRIGGER_LEVEL);
	} else {
		status = DAQmxDisableStartTrig(task->handle);
	}
	if (status != 0) {
		char buf[NIDAQ_ERR_BUFFER_SIZE];
		DAQmxGetExtendedErrorInfo(buf, NIDAQ_ERR_BUFFER_SIZE);
		printf("error: %s\n", buf);
		free_task(task);
		return NULL;
	}
	printf("Task initialized:\n");
	printf("  %d channels\n", NIDAQ_NUM_CHANNELS);
	printf("  Length: %0.2f\n", task->length);
	printf("  %lu total samples\n", task->nsamples);
	printf("  Trigger: %s\n", task->trigger);
	printf("  ADC range: %0.2f\n", task->adc_range);
	printf("  Block size: %d\n", task->block_size);
	printf("  Date: %s\n", task->date);

	return task;
}

int16_t *get_next_data_block(task_t *task) {
	/* Allocate buffer */
	uInt32 buffer_size = NIDAQ_NUM_CHANNELS * task->block_size;
	int16_t *data = calloc(buffer_size, sizeof(int16_t));
	if (data == NULL) {
		return NULL;
	}

	/* Request samples from NIDAQ */
	int32 samples_read = 0;
	int status = DAQmxReadBinaryI16(task->handle, task->block_size,
		NIDAQ_TIMEOUT, NIDAQ_FILL_MODE, data, buffer_size, &samples_read, NULL);
	if (status < 0) {
		char buf[NIDAQ_ERR_BUFFER_SIZE];
		DAQmxGetExtendedErrorInfo(buf, NIDAQ_ERR_BUFFER_SIZE);
		printf("error: %s\n", buf);
		free_task(task);
		free(data);
		return NULL;
	}
	return data;
	
}

void free_task(task_t *task) {
	DAQmxStopTask(task->handle);
	DAQmxClearTask(task->handle);
	free(task->trigger);
	free(task);
}

