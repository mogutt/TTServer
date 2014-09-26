CC = g++

INC = ../base
OPT = -Wall -Wno-deprecated -g

OBJS = FileConn.o file_server.o ../base/libbase.a 

OS_NAME = $(shell uname)
LC_OS_NAME = $(shell echo $(OS_NAME) | tr '[A-Z]' '[a-z]')
ifeq ($(LC_OS_NAME), darwin)
LIBS = -lpthread
else 
LIBS = -lpthread -luuid
endif


SERVER = file_server

all: $(SERVER)

$(SERVER): $(OBJS)
	$(CC)  $(OPT) -o $@ $(OBJS) $(LIBS)

FileConn.o: FileConn.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<
	
file_server.o: file_server.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

clean:
	rm -f $(SERVER) *.o

