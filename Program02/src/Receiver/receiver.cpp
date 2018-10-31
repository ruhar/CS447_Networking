#include <iostream>
#include <cstring>
#include <string>
#include "receiver.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <stdio.h>
#include <thread>
#include <unistd.h>

using namespace std;
using namespace cs447;

const int BUFFERSIZE = 10;
void cs447::Hello()
{
    cout<<"Welcome to the electronic age Captain!\n";
}
void cs447::Goodbye()
{
    cout<<"Thank you for using Dr. Calculus's mail services!\n";    
}
void cs447::RTSPReceiverClient(int _ServerPort)
{
    struct sockaddr_in saddress;
    saddress.sin_family = AF_INET;
    saddress.sin_addr.s_addr = htonl(INADDR_ANY);
    saddress.sin_port = htons(_ServerPort);
    int sck = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sck < 0)
    {
        throw runtime_error("Unable to start UDP socket on " + to_string(_ServerPort));
    }
    int sckbind = bind(sck,(struct sockaddr *) &saddress, sizeof(saddress));
    if (sckbind < 0)
    {
        throw runtime_error("Unable to bind to socket " + to_string(sck));
    }
    bool listen = true;
    char buffer[BUFFERSIZE];
    memset(buffer,0,BUFFERSIZE);
    struct sockaddr_in caddress;
    socklen_t clength = sizeof(caddress);
    cout<<"Waiting for data..."<<endl;
    while(listen)
    {
        int rcvdmsglength = recvfrom(sck, buffer, BUFFERSIZE, 0,(struct sockaddr *) &caddress, &clength);
        string rcvdmsg(buffer);
        while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
        {
            memset(buffer,0,BUFFERSIZE);
            rcvdmsglength = recvfrom(sck, buffer, BUFFERSIZE, 0,(struct sockaddr *) &caddress, &clength);
            rcvdmsg += buffer;
        }
        memset(buffer,0,BUFFERSIZE);
        cout<<rcvdmsg<<endl;
    }
}