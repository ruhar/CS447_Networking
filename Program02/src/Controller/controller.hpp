#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include "tcpargs.hpp" 
#include "rtspheaders.hpp"

namespace cs447
{
    void Hello();
    void Help();
    void Goodbye();
    void RTSPControlClient(std::string _ServerAddress, int _ServerPort, int _ReceiverPort);
    void RTSPReceiver(tcpargs _TCPArguments);
    void RTSPSender(tcpargs _TCPArguments, int _ReceiverPort);
}