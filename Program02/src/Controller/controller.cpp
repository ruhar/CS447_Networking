#include <iostream>
#include <cstring>
#include <string>
#include "controller.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdexcept>
#include <stdio.h>
#include <thread>
#include <unistd.h>

using namespace std;
using namespace cs447;

const int BUFFERSIZE = 64;

void cs447::Hello()
{
    cout<<"Welcome to the electronic age Captain!\n";
}
void cs447::Goodbye()
{
    cout<<"Thank you for using Dr. Calculus's mail services!\n";    
}
void cs447::RTSPControlClient(std::string _ServerAddress, int _ServerPort)
{
    struct sockaddr_in saddress;
    saddress.sin_family = AF_INET;
    saddress.sin_addr.s_addr = inet_addr(_ServerAddress.c_str());
    saddress.sin_port = htons(_ServerPort);
    int sck = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sck < 0)
    {
        throw runtime_error("Unable to start socket");
    }
    int sckconnect = connect(sck,(struct sockaddr *) &saddress,sizeof(saddress));
    if(sckconnect < 0)
    {
        throw runtime_error("Unable to connect to server: " + _ServerAddress + " Port: " + to_string(_ServerPort));
    }
    tcpargs serverinfo;
    serverinfo.socket = sck;
    serverinfo.address = saddress;
    thread sendthread (RTSPSender,serverinfo);
    thread rcvthread (RTSPReceiver,serverinfo);

    sendthread.join();
    rcvthread.detach();
    rcvthread.~thread();
    close(sck);
}
void cs447::RTSPSender(tcpargs _TCPArguments)
{
    int socket = _TCPArguments.socket;
    string input = "";
    while(input != "teardown\r\n")
    {
        input = "";
        getline(cin,input);
        input += "\r\n";
        int length = input.length();
        char buffer[length + 1];
        strcpy(buffer,input.c_str());
        send(socket,buffer,length,0);
    }
}
void cs447::RTSPReceiver(tcpargs _TCPArguments)
{
    bool listening = true;
    int socket = _TCPArguments.socket;
    char buffer[BUFFERSIZE];
    memset(buffer, 0, BUFFERSIZE);
    while(listening)
    {
        int rcvdmsglength;
        try
        {
            rcvdmsglength = recv(socket,buffer,BUFFERSIZE,0);
        }
        catch(exception er)
        {
            cout<<er.what();
        }
        string rcvdmsg(buffer);
        while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
        {
            memset(buffer, 0, BUFFERSIZE);
            rcvdmsglength = recv(socket,buffer,BUFFERSIZE,0);
            rcvdmsg += buffer;
        }
        memset(buffer, 0, BUFFERSIZE);
        cout<<rcvdmsg;
    }
}