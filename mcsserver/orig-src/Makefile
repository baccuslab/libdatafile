CC = gcc

# NI-DAQmx include and library path information
DAQ_PATH = /c/Program\ Files\ \(x86\)/National\ Instruments/NI-DAQ/DAQmx\ ANSI\ C\ Dev
DAQ_INCLUDE_PATH = $(DAQ_PATH)/include
DAQ_LIB_PATH = $(DAQ_PATH)/lib/msvc

# Windows API include and library path information
SDK_PATH = /c/Program\ Files\ \(x86\)/Windows\ Kits/8.1/Include
MINGW_INCLUDE_PATH = /mingw/include/
SDK_LIB_PATH = $(SDK_PATH)/Lib/winv6.3/um/x86/
LDFLAGS = -L$(DAQ_LIB_PATH) -L$(SDK_LIB_PATH) -lWS2_32 -lWSock32 -lnidaqmx
PROGRAM = daqsrv

DEBUG_CFLAGS = -O0 -Wall -Wextra
RELEASE_CFLAGS = -O2 -Wno-format
CFLAGS = -I$(DAQ_INCLUDE_PATH) -I$(MINGW_INCLUDE_PATH)

.PHONY: all

release: all

all: messages nidaq
	$(CC) -c -g $(RELEASE_CFLAGS) $(CFLAGS) -o $(PROGRAM).o $(PROGRAM).c
	$(CC) $(PROGRAM).o nidaq.o messages.o $(LDFLAGS) -o $(PROGRAM)

messages: messages.c messages.h
	$(CC) -c -g $(RELEASE_CFLAGS) $(CFLAGS) -o $@.o $@.c

nidaq: nidaq.c nidaq.h
	$(CC) -c -g $(RELEASE_CFLAGS) $(CFLAGS) -o $@.o $@.c 

debug: debug_all

debug_all: debug_messages debug_nidaq
	$(CC) -c -g $(DEBUG_CFLAGS) $(CFLAGS) -o $(PROGRAM).o $(PROGRAM).c
	$(CC) $(PROGRAM).o nidaq.o messages.o $(LDFLAGS) -o $(PROGRAM)

debug_messages: messages.c messages.h
	$(CC) -c -g $(DEBUG_CFLAGS) $(CFLAGS) -o messages.o messages.c

debug_nidaq: nidaq.c nidaq.h
	$(CC) -c -g $(DEBUG_CFLAGS) $(CFLAGS) -o nidaq.o nidaq.c 

clean:
	rm -f $(PROGRAM).exe
	rm -f *.o

