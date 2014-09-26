CC = g++

INC = ../base
OPT = -Wall -Wno-deprecated -g

OBJS = RouteConn.o route_server.o ../base/libbase.a 

SERVER = route_server

all: $(SERVER)

$(SERVER): $(OBJS)
	$(CC)  $(OPT) -o $@ $(OBJS) -lpthread 

	
RouteConn.o: RouteConn.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

route_server.o: route_server.cpp
	$(CC) -I $(INC) $(OPT) -c -o $@ $<

clean:
	rm -f $(SERVER) *.o

