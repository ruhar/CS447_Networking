INC = -I ./src/Common -I ./src/Server -I ./src/ControlClient -I ./src/Receiver
all: dirs server controller receiver help

help:
	@echo "Run:"
	@echo "From the directory with *.app files in the following order"
	@echo "Server"
	@echo "./server.app <server-port> <oxygen.bin> <temperature.bin> <pressure.bin>"
	@echo ""
	@echo "Receiver"
	@echo "./receiver.app <receiver-port>"
	@echo ""
	@echo "Controller"
	@echo "./controller.app <hostname/server-ip> <server-port> <receiver-port>"
	@echo ""
	@echo "Example"
	@echo "*Assuming sensor files are in same directory as executables"
	@echo "./server.app 8100 ./oxygen.bin ./temperature.bin ./pressure.bin"
	@echo "./receiver.app 4800"
	@echo "./controller.app home.cs.siue.edu 8100 4800"

dirs:
	mkdir -p ./bin
	mkdir -p ./bin/server
	mkdir -p ./bin/controller
	mkdir -p ./bin/receiver
	mkdir -p ./.passwords
	mkdir -p ./.logs

server: sensorcontrolclient.o sensorcontrol.o rtspheaders.o tcpargs.o common.o server.o servermain.o 
	g++ -g -pthread ./bin/server/sensorcontrolclient.o ./bin/server/sensorcontrol.o ./bin/rtspheaders.o ./bin/tcpargs.o ./bin/common.o ./bin/server/server.o ./bin/server/main.o -o ./server.app

controller: rtspheaders.o tcpargs.o common.o controller.o controllermain.o
	g++ -g -pthread ./bin/tcpargs.o ./bin/common.o ./bin/controller/controller.o ./bin/controller/main.o -o ./controller.app

receiver: common.o receiver.o receivermain.o
	g++ -g -pthread ./bin/common.o ./bin/receiver/receiver.o ./bin/receiver/main.o -o ./receiver.app

servermain.o: src/Server/main.cpp src/Common/common.hpp
	g++ -c -Wall -g $(INC) src/Server/main.cpp -o ./bin/server/main.o

server.o: src/Server/server.cpp src/Server/server.hpp
	g++ -c -Wall -g $(INC) src/Server/server.cpp -o ./bin/server/server.o

controllermain.o: src/Controller/main.cpp src/Common/common.hpp
	g++ -c -Wall -g $(INC) src/Controller/main.cpp -o ./bin/controller/main.o

controller.o: src/Controller/controller.cpp src/Controller/controller.hpp
	g++ -c -Wall -g $(INC) src/Controller/controller.cpp -o ./bin/controller/controller.o

receivermain.o: src/Receiver/main.cpp src/Common/common.hpp
	g++ -c -Wall -g $(INC) src/Receiver/main.cpp -o ./bin/receiver/main.o

receiver.o: src/Receiver/receiver.cpp src/Receiver/receiver.hpp
	g++ -c -Wall -g $(INC) src/Receiver/receiver.cpp -o ./bin/receiver/receiver.o

common.o: src/Common/common.cpp src/Common/common.hpp
	g++ -c -Wall -g $(INC) src/Common/common.cpp -o ./bin/common.o

tcpargs.o: src/Common/tcpargs.cpp src/Common/tcpargs.hpp
	g++ -c -Wall -g $(INC) src/Common/tcpargs.cpp -o ./bin/tcpargs.o

rtspheaders.o: src/Common/rtspheaders.cpp src/Common/rtspheaders.hpp
	g++ -c -Wall -g $(INC) src/Common/rtspheaders.cpp -o ./bin/rtspheaders.o

sensorcontrol.o: src/Server/SensorControl.cpp src/Server/SensorControl.hpp
	g++ -c -Wall -g $(INC) src/Server/SensorControl.cpp -o ./bin/server/sensorcontrol.o

sensorcontrolclient.o: src/Server/SensorControlClient.cpp src/Server/SensorControlClient.hpp
	g++ -c -Wall -g $(INC) src/Server/SensorControlClient.cpp -o ./bin/server/sensorcontrolclient.o

clean:
	rm ./server.app ./receiver.app ./controller.app
	rm -r ./bin/
	rm -r ./.passwords/
	rm -r ./.logs/
	
