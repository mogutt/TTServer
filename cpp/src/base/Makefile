CC = g++

OPT = -Wall -Wno-deprecated -g

OBJS = UtilPdu.o util.o ConfigFileReader.o BaseSocket.o EventDispatch.o netlib.o ImPduBase.o ImPduClient.o \
	ImPduServer.o ImPduFile.o ImPduGroup.o imconn.o ServInfo.o 

LIBBASE = libbase.a

all: $(LIBBASE)

$(LIBBASE): $(OBJS)
	ar -r $@ $(OBJS)

UtilPdu.o: UtilPdu.cpp
	$(CC) $(OPT) -c -o $@ $<
	
util.o: util.cpp
	$(CC) $(OPT) -c -o $@ $<

ConfigFileReader.o: ConfigFileReader.cpp
	$(CC) $(OPT) -c -o $@ $<
	
BaseSocket.o: BaseSocket.cpp
	$(CC) $(OPT) -c -o $@ $<

EventDispatch.o: EventDispatch.cpp
	$(CC) $(OPT) -c -o $@ $<

netlib.o: netlib.cpp 
	$(CC) $(OPT) -c -o $@ $<

imconn.o: imconn.cpp
	$(CC) $(OPT) -c -o $@ $<

ImPduBase.o: ImPduBase.cpp
	$(CC) $(OPT) -c -o $@ $<

ImPduClient.o: ImPduClient.cpp
	$(CC) $(OPT) -c -o $@ $<

ImPduServer.o: ImPduServer.cpp
	$(CC) $(OPT) -c -o $@ $<

ImPduFile.o: ImPduFile.cpp
	$(CC) $(OPT) -c -o $@ $<

ImPduGroup.o: ImPduGroup.cpp
	$(CC) $(OPT) -c -o $@ $<

ServInfo.o: ServInfo.cpp
	$(CC) $(OPT) -c -o $@ $<


clean:
	rm -f $(LIBBASE) *.o

