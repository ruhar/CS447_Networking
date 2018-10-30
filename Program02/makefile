#makefile for server.cpp in P02
# all: server client
INC = -I ./src/Common -I ./src/Server -I ./src/ControlClient
all: server client

server: rtspheaders.o tcpargs.o common.o server.o servermain.o 
	g++ -g -pthread ./bin/rtspheaders.o ./bin/tcpargs.o ./bin/common.o ./bin/server/server.o ./bin/server/main.o -o ./server.app

client: rtspheaders.o tcpargs.o common.o client.o clientmain.o
	g++ -g -pthread ./bin/tcpargs.o ./bin/common.o ./bin/control/client.o ./bin/control/main.o -o ./client.app

servermain.o: src/Server/main.cpp src/Common/common.hpp
	g++ -c -Wall -g $(INC) src/Server/main.cpp -o ./bin/server/main.o

server.o: src/Server/server.cpp src/Server/server.hpp
	g++ -c -Wall -g $(INC) src/Server/server.cpp -o ./bin/server/server.o

clientmain.o: src/ControlClient/main.cpp src/Common/common.hpp
	g++ -c -Wall -g $(INC) src/ControlClient/main.cpp -o ./bin/control/main.o

client.o: src/ControlClient/controlclient.cpp src/ControlClient/controlclient.hpp
	g++ -c -Wall -g $(INC) src/ControlClient/controlclient.cpp -o ./bin/control/client.o

common.o: src/Common/common.cpp src/Common/common.hpp
	g++ -c -Wall -g $(INC) src/Common/common.cpp -o ./bin/common.o

tcpargs.o: src/Common/tcpargs.cpp src/Common/tcpargs.hpp
	g++ -c -Wall -g $(INC) src/Common/tcpargs.cpp -o ./bin/tcpargs.o

rtspheaders.o: src/Common/rtspheaders.cpp src/Common/rtspheaders.hpp
	g++ -c -Wall -g $(INC) src/Common/rtspheaders.cpp -o ./bin/rtspheaders.o

clean:
	rm ./bin/*.* ./bin/control/*.* ./bin/server/*.* ./server.app

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