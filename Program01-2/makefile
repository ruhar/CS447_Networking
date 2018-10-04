#makefile for server.cpp in P01
# all: server client
#all: server

.server: main.app
	g++ -pthread -Wall -g -o ./server/main.app ./server/main.o ./server/server.o ./common/common.o
	
./server/main.o: ./server/main.cpp
	g++ -Wall -g -c ./server/main.cpp

./server/server.o: ./server/server.cpp
	g++ -Wall -g -c ./server/server.cpp

# client: clientmain.o client.o
# 	g++ -Wall -g -o ./client/main.app ./client/clientmain.o ./client/client.o
	
# clientmain.o: main.cpp
# 	g++ -Wall -g -c ./client/main.cpp

# client.o: client.cpp
# 	g++ -Wall -g -c ./client/client.cpp

./common/common.o: ./common/common.cpp
	g++ -Wall -g -c ./common/common.cpp