INCLUDE_DIR = ./include
SRC_DIR = ./src
LIB_DIR = ./lib

CC = g++
DEBUG_FLAGS = -g -O0 -Wall -Wextra
CXXFLAGS = -std=c++11 -stdlib=libc++ -O0 -g -I$(INCLUDE_DIR) -Wno-c++11-extensions
LDFLAGS = -lhdf5 -lhdf5_cpp -lboost_container

.PHONY: all

all: h5recording

h5recording: 
	$(CC) -c -fPIC $(CXXFLAGS) -o $(LIB_DIR)/$@.o $(SRC_DIR)/$@.cpp
	$(CC) -shared $(LDFLAGS) $(LIB_DIR)/$@.o -o lib$@.dylib
	mv lib$@.dylib ./lib/

clean:
	rm ./lib/*.o ./lib/*.dylib ./lib/*.so

install: h5recording
	ln -sf $(shell pwd)/lib/libh5recording.dylib /usr/local/lib/
