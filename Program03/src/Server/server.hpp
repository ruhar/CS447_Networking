#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include "tcpargs.hpp" 
#include "rtspheaders.hpp"
#include <bitset>
#include "SensorControlClient.hpp"


namespace cs447
{
    void Hello();
    void Goodbye();
    void RTSPServer(int _Port, std::string _OxygenFile, std::string _TemperatureFile, std::string _PressureFile);
    int RTSPSendResponse(int &_ClientSocket, int _ResponseCode, RTSPHeaders &_Headers, cs447::HEADER _Header, std::string _ServerIP);
    int DeliverEmail(std::string _Email[],std::string _Path);
    void RTSPServerHandler(tcpargs _sckinfo);
    void UDPServer(int _Port);
    int UDPSendResponse(int *_ServerSocket,struct sockaddr_in _ClientAddr,std::string _Message);
    int RetrieveEmail(std::vector<std::string> &_Emails,std::vector<std::string> &_EmailFilenames,std::string _Mailbox, int _Count);
    int SaveRetrievedEmail(std::string _Email, std::string _Path, std::string _Filename);
    void ReadOxygenSensor(std::vector<std::bitset<5>> &_SensorData, std::string _FileName, int &_MaxReadings);
    void ReadTemperatureSensor(std::vector<std::bitset<8> > &_SensorData, std::string _FileName, int &_MaxReadings);
    void ReadPressureSensor(std::vector<std::bitset<11> > &_SensorData, std::string _FileName, int &_MaxReadings);
    void RTSPPlay(int _Socket, std::string _ReceiverIP, int _ReceiverPort);
    void SensorIncrement(bool &_Playing, int &_Counter, int &_Size);

}