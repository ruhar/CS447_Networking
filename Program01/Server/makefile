#makefile for server.cpp in P01
main: main.o server.o
	g++ -pthread -Wall -g -o main.app main.o server.o
	
main.o: main.cpp
	g++ -Wall -g -c main.cpp

server.o: server.cpp
	g++ -Wall -g -c server.cpp

