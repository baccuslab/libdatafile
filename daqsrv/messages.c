/* messages.c
 * Implementation of marshalling/unmarshalling of message formats on the wire.
 * 
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include "messages.h"

int read_msg_header(char *data, msg_t *msg_type, uint32_t *msg_size) {
	*msg_type = ntohl(*(msg_t *) data);
	*msg_size = ntohl(*(uint32_t *) (data + sizeof(msg_t)));
	return 0;
} 

init_msg_t *parse_init_msg(char *data) {
	init_msg_t *msg = calloc(1, sizeof(init_msg_t));
	if (msg == NULL)
		return NULL;
	char *ptr = data;
	msg->msg_type = ntohl(*(msg_t *) ptr);
	ptr += sizeof(msg_t);

	msg->msg_size = ntohl(*(uint32_t *) ptr);
	ptr += sizeof(uint32_t);

	msg->trigger_length = ntohl(*(uint32_t *) ptr);
	ptr += sizeof(uint32_t);

	msg->trigger = calloc(msg->trigger_length + 1, 1);
	if (msg->trigger == NULL) {
		free(msg);
		return NULL;
	}
	strncpy(msg->trigger, ptr, msg->trigger_length);
	ptr += msg->trigger_length;

	uint32_t tmp = ntohl(*(uint32_t *) ptr);
	msg->expt_length = *(float *) &tmp;
	ptr += sizeof(float);

	tmp = ntohl(*(uint32_t *) ptr);
	msg->adc_range = *(float *) &tmp;
	ptr += sizeof(float);

	msg->block_size = ntohl(*(uint32_t *) ptr);
	return msg;
}

void free_init_msg(init_msg_t *msg) {
	if (msg == NULL)
		return;
	free(msg->trigger);
	free(msg);
}

params_msg_t *create_params_msg(task_t *task) {
	params_msg_t *msg = calloc(1, sizeof(params_msg_t));
	if (msg == NULL)
		return NULL;
	msg->msg_type = EXPT_PARAMS;
	msg->msg_size = sizeof(params_msg_t);
	msg->nseconds = task->length;
	msg->nsamples = task->nsamples;
	msg->sample_rate = task->sample_rate;
	msg->adc_range = task->adc_range;
	msg->adc_resolution = task->adc_resolution;
	msg->block_size = task->block_size;
	msg->nchannels = task->nchannels;
	msg->trigger_length = task->trigger_length;
	msg->trigger = calloc(msg->trigger_length, 1);
	if (msg->trigger == NULL) {
		free(msg);
		return NULL;
	}
	strncpy(msg->trigger, task->trigger, msg->trigger_length);
	msg->date_length = task->date_length;
	msg->date = calloc(msg->date_length, 1);
	if (msg->date == NULL) {
		free(msg);
		free(msg->trigger);
		return NULL;
	}
	strncpy(msg->date, task->date, msg->date_length);
	return msg;
}

void free_params_msg(params_msg_t *msg) {
	if (msg == NULL)
		return;
	free(msg->trigger);
	free(msg->date);
	free(msg);
}

data_msg_t *create_data_msg(task_t *task, int16_t *data) {
	data_msg_t *msg = calloc(1, sizeof(data_msg_t));
	if (msg == NULL)
		return NULL;
	msg->msg_type = DATA_CHUNK;
	msg->nchannels = task->nchannels;
	msg->nsamples = task->block_size;
	/* Data is now owned by message, do not free separately */
	msg->data = data;
	return msg;
}

void free_data_msg(data_msg_t *msg) {
	free(msg->data);
	free(msg);
}

err_msg_t *create_err_msg(uint32_t err_msg_size, char *err_msg) {
	err_msg_t *msg = calloc(1, sizeof(err_msg_t));
	if (msg == NULL)
		return NULL;
	msg->msg_type = ERROR_MSG;
	msg->err_msg_size = err_msg_size;
	msg->err_msg = calloc(msg->err_msg_size + 1, 1);
	if (msg->err_msg == NULL) {
		free(msg);
		return NULL;
	}
	msg->err_msg = strncpy(msg->err_msg, err_msg, err_msg_size);
	return msg;
}

