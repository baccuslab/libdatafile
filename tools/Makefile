SRC_DIR = ./src
BIN_DIR = ./bin
LIB_DIR = ./lib
INCLUDE_DIR = ./include

CC = gcc
#CFLAGS = -O0 -Wall -Wextra -g # uncomment to make debug build
CFLAGS = -O2 -I$(INCLUDE_DIR)
LDFLAGS = -lhdf5
LIB_FLAGS = -L$(LIB_DIR) -lmeatools

.PHONY: all

all: lib bin2hdf hdf2bin

bin2hdf: lib
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIB_FLAGS) -o $(BIN_DIR)/$@ $(SRC_DIR)/$@.c

hdf2bin: lib
	$(CC) $(CFLAGS) $(LDFLAGS) $(LIB_FLAGS) -o $(BIN_DIR)/$@ $(SRC_DIR)/$@.c

lib: bintools hdftools
	$(CC) -shared $(LDFLAGS) $(LIB_DIR)/bintools.o $(LIB_DIR)/hdftools.o -o $(LIB_DIR)/libmeatools.so 

bintools: $(SRC_DIR)/bintools.c $(INCLUDE_DIR)/bintools.h
	$(CC) -c -fPIC $(CFLAGS) -o $(LIB_DIR)/$@.o $(SRC_DIR)/$@.c

hdftools: $(SRC_DIR)/hdftools.c $(INCLUDE_DIR)/hdftools.h
	$(CC) -c -fPIC $(CFLAGS) -o $(LIB_DIR)/$@.o $(SRC_DIR)/$@.c

clean:
	rm -rf *.dSYM
	rm -f $(LIB_DIR)/*.o
	rm -f $(LIB_DIR)/*.so
	rm -f bin/*

