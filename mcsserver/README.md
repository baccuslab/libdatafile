`daqsrv`
========

A NI-DAQ server program.
(C) 2015 Benjamin Naecker bnaecker@stanford.edu

Overview
--------

National Instruments data acquisition devices are generally controlled through
the DAQmx C driver API. However, in general, NI provides far better support for
Windows machines, and thus this driver set does not run on Mac or Linux 
machines. Thus software directly interfacing with NI devices should run on Windows
for optimal performance. But Windows sucks.

The `daqsrv` application is a small server application, which exposes a minimal
subset of the DAQmx API over the network. This allows client applications, 
running remotely, to initialize, run, query, or stop a data acquisition (a 
"task" in DAQmx parlance). This does the minimum amount necessary to free us
of the Windows constraint imposed by NI.

C/C++
-----

The `daqsrv` application was originally written in pure `C`, using the Winsock
API for network programming. However, the lab has decided that this application
should take a more prominent role in the recording software chain. For example,
we would like to have this application itself (or a tightly integrated client)
saving the data locally on the Windows machine, rather than having the only
copy of the data streaming over the network. Also, the `daqsrv` application should
support multiple simultaneous clients, all of which may connect/disconnect
at will and receive data for, e.g., plotting (MeaLog), online-analysis, etc.

Thus the program has been rewritten in `C++`, using the Qt application framework,
so that I don't have to use the ridiculous Winsock library.

Components
----------

- `main.cc`
	- The main application entry point
- `nidaq.cc`
	- Library representing NI-DAQ tasks and wrapping the driver functionality
- `serverwindow.cc`
	- Implementation of the main GUI window for controlling the server program
- `daqsrv.cc`
	- The implementation of the server itself, receiving and responding to 
	messages, managing clients, etc.

Usage
-----

Compile and run the `daqsrv` application on any Windows machine that connects
to a DAQmx-capable NI device. The application consists of a GUI for controlling
the National Instruments card itself, and also shows information about any
running recording, as well as connected clients.

Client applications connect to the server via port 12345. 

fakeserver.py
-------------

This is a small Python emulation of the actual server. Since the actual C
code for the server can only be compiled on Windows, this program allows
users to develop software on other platforms.

