#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include "tcpargs.hpp" 
#include "rtspheaders.hpp"
#include <bitset>

namespace cs447
{
    void Hello();
    void Goodbye();
    void SMTPServer(int _Port);
    void RTSPServer(int _Port, std::string _OxygenFile, std::string _TemperatureFile, std::string _PressureFile);
    bool SMTPHelo(int &_ClientSocket, sockaddr_in _ClientAddress);
    void SMTPMailFrom();
    int SMTPSendResponse(int &_ClientSocket, int _ResponseCode, std::string _Message="");
    int RTSPSendResponse(int &_ClientSocket, int _ResponseCode, rtspheaders _Headers);
    int DeliverEmail(std::string _Email[],std::string _Path);
    void *SMTPServerHandler(void *_sckinfo);
    void *RTSPServerHandler(void *_sckinfo);
    void UDPServer(int _Port);
    int UDPSendResponse(int *_ServerSocket,struct sockaddr_in _ClientAddr,std::string _Message);
    void stringSplit(std::string _InputToSplit, std::vector<std::string> &_DelimitedOutput, char _Delimiter);
    int RetrieveEmail(std::vector<std::string> &_Emails,std::vector<std::string> &_EmailFilenames,std::string _Mailbox, int _Count);
    int SaveRetrievedEmail(std::string _Email, std::string _Path, std::string _Filename);
    void ReadOxygenSensor(std::vector<std::bitset<5>> &_SensorData, std::string _FileName);
    void ReadTemperatureSensor(std::vector<std::bitset<8> > &_SensorData, std::string _FileName);
    void ReadPressureSensor(std::vector<std::bitset<11> > &_SensorData, std::string _FileName);

}