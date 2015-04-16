.PHONY = all

all: logserver

logserver:
	protoc --cpp_out=. logserver.proto

clean:
	rm -f *.cc *.h
