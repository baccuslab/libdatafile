Mealog
======

Application to log data from an MEA experiment.

Messages
========

The Mealog application starts up a TCP server on port 44444 that can be
queried for information about the status of the recording. Messages should
be sent as Google Protocol Buffer messages as defined in the `./proto`
directory. The raw binary data of the messages should be prepended with
the message size, as an unsigned 32 bit integer. 

In pseudocode:

	socket.connect("localhost", 44444)
	socket.send(msg.ByteSize())
	socket.send(msg)

Dependencies
============

- C++11
- Qt5
- HDF5
- Boost
- Google Protocol Buffers
