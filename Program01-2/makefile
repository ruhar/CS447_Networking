#makefile for server.cpp in P01
# all: server client
INC = -I ./src/Common -I ./src/Server -I ./src/Client
all: server

server: smtpargs.o common.o server.o main.o 
	g++ -g -pthread ./bin/smtpargs.o ./bin/common.o ./bin/server.o ./bin/main.o -o ./server.app

main.o: src/Server/main.cpp src/Common/common.hpp
	g++ -c -Wall -g $(INC) src/Server/main.cpp -o ./bin/main.o

server.o: src/Server/server.cpp src/Server/server.hpp
	g++ -c -Wall -g $(INC) src/Server/server.cpp -o ./bin/server.o

common.o: src/Common/common.cpp src/Common/common.hpp
	g++ -c -Wall -g $(INC) src/Common/common.cpp -o ./bin/common.o

smtpargs.o: src/Common/smtpargs.cpp src/Common/smtpargs.hpp
	g++ -c -Wall -g $(INC) src/Common/smtpargs.cpp -o ./bin/smtpargs.o

clean:
	rm ./bin/*.* ./server.app

# g++ -o ./bin/main.app $(INC) ./src/Common/common.cpp ./src/Server/server.cpp ./src/Server/main.cpp
# server: main.app
# 	g++ -pthread -Wall -g -o /server/main.app ./server/main.o ./server/server.o ./common/common.o
	
# /server/main.o: /server/main.cpp
# 	g++ -Wall -g -c /server/main.cpp

# /server/server.o: /server/server.cpp
# 	g++ -Wall -g -c /server/server.cpp

# # client: clientmain.o client.o
# # 	g++ -Wall -g -o ./client/main.app ./client/clientmain.o ./client/client.o
	
# # clientmain.o: main.cpp
# # 	g++ -Wall -g -c ./client/main.cpp

# # client.o: client.cpp
# # 	g++ -Wall -g -c ./client/client.cpp

# ./common/common.o: ./common/common.cpp
# 	g++ -Wall -g -c ./common/common.cpp