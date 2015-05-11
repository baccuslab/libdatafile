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

Components
----------

- `daqsrv.c` is the main application event loop, which runs the server itself
- `messages.c` describes the set of messages that can be sent/received, i.e.
the actual API that the application exports.
- `nidaq.c` wraps the DAQmx API and provides a simplified interface.

Usage
-----

Compile and run the `daqsrv` application on any Windows machine that connects
to a DAQmx-capable NI device. Client applications connect to the server via
port 12345. Communication is described via the messages in `messages.c`. 
In general, they're a simple pack binary blob format. The notable exception
is data. Clients initialize experiments and request parameters with messages,
but, after the experiment begins, all data is sent as a single stream of
bytes. This means the client must keep track of their position in the 
stream, and remember how many samples were requested and how many channels
exist in the stream. The reason for this is simplicity and efficiency. This
allows clients to read as much data as they want at a time, without regard
to message blocks.

fakeserver.py
-------------

This is a small Python emulation of the actual server. Since the actual C
code for the server can only be compiled on Windows, this program allows
users to develop software on other platforms.

