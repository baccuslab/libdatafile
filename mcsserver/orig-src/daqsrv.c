/* daqsrv.c 
 * This file implements the basic networking and server interface in the 
 * NI-DAQ server application.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#include "daqsrv.h"
#include "messages.h"
#include "nidaq.h"

/* Globals are evil ... */

void print_usage_and_exit(void) {
	printf("NI-DAQ server program\n");
	exit(EXIT_SUCCESS);
}

int init_server(struct addrinfo *server_addr, SOCKET *server_socket) {
	printf("Initializing NI-DAQ server...\n");
	/* Get address information for the server */
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	int result = getaddrinfo(NULL, PORT, &hints, &server_addr);
	if (result != 0) {
		printf("Could not get address info: %d\n", result);
		WSACleanup();
		return -1;
	}

	/* Create socket for the server */
	*server_socket = socket(server_addr->ai_family, 
		server_addr->ai_socktype, server_addr->ai_protocol);
	if (*server_socket == INVALID_SOCKET) {
		printf("Could not create server socket: %d\n", 
			WSAGetLastError());
		freeaddrinfo(server_addr);
		WSACleanup();
		return -1;
	}

	/* Bind and listen */
	result = bind(*server_socket, server_addr->ai_addr, 
		(int) server_addr->ai_addrlen);
	if (result == SOCKET_ERROR) {
		printf("bind failed: %d\n", WSAGetLastError());
		freeaddrinfo(server_addr);
		closesocket(*server_socket);
		WSACleanup();
		return -1;
	}
	result = listen(*server_socket, 1);
	if (result == SOCKET_ERROR) {
		printf("listen failed: %d\n", WSAGetLastError());
		freeaddrinfo(server_addr);
		closesocket(*server_socket);
		WSACleanup();
		return -1;
	}
	return 0;
}

int accept_client(SOCKET server, SOCKET *client, 
		struct sockaddr_in *client_addr) {
	printf("Waiting for clients ...\n");
	int addrlen = sizeof(struct sockaddr_in);
	*client = accept(server, (struct sockaddr *) client_addr, &addrlen);
	if (*client == INVALID_SOCKET) {
		closesocket(server);
		WSACleanup();
		return -1;
	}
	printf("Accepted client at %s:%d\n", 
		inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
	return 0;
}

int send_params_msg(SOCKET client, params_msg_t *msg) {
	int size_minus_ptrs = sizeof(params_msg_t) - 2 * sizeof(char *);
	int buffer_size = (size_minus_ptrs + 
		msg->trigger_length + sizeof(uint32_t) + msg->date_length);
	char *buffer = calloc(buffer_size, 1);
	if (buffer == NULL)
		return -1;
	memcpy(buffer, msg, size_minus_ptrs); // Up to and including trig len
	memcpy(buffer + size_minus_ptrs, 
		msg->trigger, msg->trigger_length);
	memcpy(buffer + size_minus_ptrs + msg->trigger_length, 
		&(msg->date_length), sizeof(uint32_t));
	memcpy(buffer + size_minus_ptrs + 
		msg->trigger_length + sizeof(uint32_t),
		msg->date, msg->date_length);
	int nsent = send(client, buffer, buffer_size, 0);
	free(buffer);
	return nsent;
}

int send_data_msg(SOCKET client, data_msg_t *msg) {
	/*
	int buffer_size = (sizeof(data_msg_t) - sizeof(int16_t *) +
		msg->nsamples * msg->nchannels * sizeof(int16_t));	
	char *buffer = calloc(buffer_size, 1);
	if (buffer == NULL)
		return -1;
	int offset = 0;
	memcpy(buffer, &(msg->msg_type), sizeof(msg_t));
	offset += sizeof(msg_t);
	memcpy(buffer + offset, &(msg->msg_size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer + offset, &(msg->nchannels), sizeof(uint16_t));
	offset += sizeof(uint16_t);
	memcpy(buffer + offset, &(msg->nsamples), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(buffer + offset, msg->data, 
		msg->nsamples * msg->nchannels * sizeof(int16_t));
	int nsent = send(client, buffer, buffer_size, 0);
	free(buffer);
	return nsent;
	*/
	int buffer_size = msg->nsamples * msg->nchannels *sizeof(int16_t);
	char *buffer = calloc(buffer_size, 1);
	if (buffer == NULL)
		return -1;
	memcpy(buffer, msg->data, buffer_size);
	int nsent = send(client, buffer, buffer_size, 0);
	free(buffer);
	return nsent;
}

void start_experiment(task_t *task) {
	DAQmxStartTask(task->handle);
	task->running = true;
}

int recv_message(SOCKET client, task_t **task) {
	char buffer[BUFFER_SIZE] = {'\0'}; 
	int nrecv = recv(client, buffer, BUFFER_SIZE, 0);
	if (nrecv < 0)
		return -1;
	if (nrecv == 0) {
		/* Client closed connection */
		printf("Client closed remote connection, exiting.\n");
		return -1;
	}
	msg_t msg_type;
	uint32_t msg_size = 0;
	int nsent = 0;
	int status = read_msg_header(buffer, &msg_type, &msg_size);
	if (status == -1)
		return status;

	switch (msg_type) {
		case INIT_EXPERIMENT:
			if (*task != NULL) // Ignore multiple init messages
				return -1;
			printf("Received INIT message\n");
			init_msg_t *init_msg = parse_init_msg(buffer);
			*task = init_task(init_msg->trigger, 
				init_msg->block_size, init_msg->expt_length, 
				init_msg->adc_range);
			free_init_msg(init_msg);
			if (*task == NULL) 
				return -1;
			else
				return 0;

		case EXPT_PARAMS_REQ:
			if (*task == NULL) // must initialize first
				return -1;
			printf("Received PARAMS_REQ message. Responding.\n");
			params_msg_t *params_msg = create_params_msg(task);

			/* Send params message */
			nsent = send_params_msg(client, params_msg);
			free_params_msg(params_msg);
			if (nsent <= 0)
				return -1;

		case EXPT_PARAMS:
			// server never receives this message
			return 0;

		case CHECK_READY:
			return 0;

		case READY_STATUS:
			// server never receives this message
			return 0;

		case START_EXPT:
			if (*task == NULL)
				return 0;
			if ((*task)->finished || (*task)->running)
				return 0;
			printf("Received START_EXPT message. "\
				"Starting data acquisition from NI-DAQ\n");
			start_experiment(*task);
			return 0;

		case DATA_CHUNK:
			// server never receives this message
			return 0;

		case CLOSE:
			printf("Received CLOSE message\n");
			return -1;

		case ERROR_MSG:
			printf("Received ERROR message\n");
			return -1;
	}
	return 0;
	
}

bool check_task_finished(task_t *task) {
	if (task == NULL)
		return false;
	return (task->samples_collected >= task->nsamples);
}

int read_and_send_data(SOCKET client, task_t *task) { 
	int16_t *data = get_next_data_block(task);
	task->samples_collected += task->block_size;

	/* Notify of progress */
	if ((task->samples_collected % (task->nsamples / 10)) == 0) {
		printf("Acquired %lu samples.\n", 
			task->samples_collected);
	}

	data_msg_t *msg = create_data_msg(task, data);
	if (send_data_msg(client, msg) < 0) { 
		int err = WSAGetLastError();
		printf("Error sending data to client. Error code: %d\n", err);
		free_data_msg(msg);
		return -1;
	}
	if (check_task_finished(task)) {
		task->running = false;
		task->finished = true;
		printf("Task completed\n");
	}
	free_data_msg(msg);
	return 0;
} 

int run_polling_loop(SOCKET server, SOCKET client, task_t **task) {
	/* Setup polling */
	int nfds = 2;
	struct fd_set readfds, exceptfds;
	FD_ZERO(&readfds);
	FD_ZERO(&exceptfds);

	/* Setup time structure for non-blocking IO */
	const struct timeval poll_time = {
		.tv_sec = 0,
		.tv_usec = 0
	};

	/* Loop */
	int ret = 0;
	printf("Starting polling loop ...\n");
	while (ret == 0) {

		/* Read next data block and send */
		if (((*task) != NULL) && ((*task)->running))
			read_and_send_data(client, *task);
		
		/* Must update all sets each pass through loop. */
		FD_SET(server, &exceptfds);
		FD_SET(client, &readfds);
		FD_SET(client, &exceptfds);

		/* Select */
		ret = select(nfds, &readfds, NULL, &exceptfds, &poll_time);
		if (ret == SOCKET_ERROR) {
			FD_ZERO(&readfds);
			FD_ZERO(&exceptfds);
			printf("select error: %d\n", WSAGetLastError());
			closesocket(server);
			closesocket(client);
			WSACleanup();
			return -1;

		}
		if (ret == 0)
			continue;

		/* Deal with messages from the client */
		if (FD_ISSET(client, &readfds))
			ret = recv_message(client, task);

		/* Deal with exceptional conditions on server and client */
		if (FD_ISSET(client, &exceptfds))
			printf("client exception\n");

		if (FD_ISSET(server, &exceptfds))
			printf("server exception\n");

		/* Clear polling set */
		FD_ZERO(&exceptfds);
		FD_ZERO(&readfds);
	}
	return ret;
}

int main(int argc, char **argv) {

	if (argc > 1) {
		if (strcmp(argv[1], "--help")  || strcmp(argv[1], "-h"))
			print_usage_and_exit();
	}

	/* Initialize usage of winsock library */
	WSADATA wsa_data;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (ret != 0) {
		printf("WSAStartup failed: %d\n", ret);
		return EXIT_FAILURE;
	}

	/* Create and initialize the server */
	struct addrinfo *server_addr = NULL;
	SOCKET server = INVALID_SOCKET;
	if (init_server(server_addr, &server) == -1)
		return EXIT_FAILURE;
	freeaddrinfo(server_addr);

	/* Initialize task and some parameters */
	task_t *task = NULL;
	uint64_t samples_collected = 0;

	while (true) {

		/* Accept a client */
		SOCKET client = INVALID_SOCKET;
		struct sockaddr_in client_addr;
		ZeroMemory(&client_addr, sizeof(struct sockaddr_in));
		if (accept_client(server, &client, &client_addr) == -1)
			return EXIT_FAILURE;

		/* Enter a polling loop, 
		 * waiting for client messages and responding 
		 */
		ret = run_polling_loop(server, client, &task);
		
		/* Clean up resources */
		if (task != NULL)
			free_task(task);
		closesocket(client);
	}
	closesocket(server);
	WSACleanup();
	return ret;
}

