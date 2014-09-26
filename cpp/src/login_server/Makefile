CC = g++

INC = ../base
OPT = -Wall -Wno-deprecated -g

OBJS = LoginConn.o login_server.o ../base/libbase.a 

SERVER = login_server

all: $(SERVER)

$(SERVER): $(OBJS)
	$(CC)  $(OPT) -o $@ $(OBJS) -lpthread 

LoginConn.o: LoginConn.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<
	
login_server.o: login_server.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

clean:
	rm -f $(SERVER) *.o

