#makefile for client.cpp in P01
main: main.o client.o
	g++ -Wall -g -o main.app main.o client.o
	
main.o: main.cpp
	g++ -Wall -g -c main.cpp

client.o: client.cpp
	g++ -Wall -g -c client.cpp