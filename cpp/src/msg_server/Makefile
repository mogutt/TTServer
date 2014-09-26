CC = g++

INC = ../base
OPT = -Wall -Wno-deprecated -g

OBJS = FileServConn.o HttpParserWrapper.o HttpQuery.o http_parser.o HttpConn.o jsonxx.o ImUser.o AttachData.o GroupChat.o FileHandler.o MsgConn.o LoginServConn.o RouteServConn.o DBServConn.o msg_server.o ../base/libbase.a 

OS_NAME = $(shell uname)
LC_OS_NAME = $(shell echo $(OS_NAME) | tr '[A-Z]' '[a-z]')
ifeq ($(LC_OS_NAME), darwin)
LIBS = -lpthread
else 
LIBS = -lpthread -luuid
endif

SERVER = msg_server

all: $(SERVER)

$(SERVER): $(OBJS)
	$(CC)  $(OPT) -o $@ $(OBJS) $(LIBS)

GroupChat.o: GroupChat.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

FileHandler.o: FileHandler.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

MsgConn.o: MsgConn.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

LoginServConn.o: LoginServConn.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

RouteServConn.o: RouteServConn.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

DBServConn.o: DBServConn.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

msg_server.o: msg_server.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

ImUser.o: ImUser.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

AttachData.o: AttachData.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

HttpQuery.o: HttpQuery.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<
	
http_parser.o: http_parser.c
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

HttpConn.o: HttpConn.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

HttpParserWrapper.o: HttpParserWrapper.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<	

jsonxx.o: jsonxx.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

FileServConn.o: FileServConn.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

clean:
	rm -f $(SERVER) *.o
checkos:
	@echo $(LC_OS_NAME)
