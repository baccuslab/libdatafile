/* daqsrv.h
 * Header file for the NI-DAQmx server.
 *
 * (C) 2015 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef _DAQSRV_H_
#define _DAQSRV_H_

#define _WIN32_WINNT 0x0501 // Needed to get definitions of socket functions

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define BUFFER_SIZE 2048	 // Size of message recv buffer

char HOSTNAME[] = "localhost";
char PORT[] = "12345";

#endif // Include guard


