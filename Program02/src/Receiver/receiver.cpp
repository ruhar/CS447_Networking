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
#include <regex>
#include "common.hpp"
#include <vector>

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
    // cout<<"12345678901234567890 12345678901234567890 12345678901234567890"<<endl;
    cout<<"Oxygen               Temperature          Pressure"<<endl;
    cout<<"--------------------|--------------------|--------------------"<<endl;
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
        vector<string> rcvdsplit;
        StringSplit(rcvdmsg,rcvdsplit,';');
        bitset<5> oxygen;
        bitset<11> pressure;
        bitset<8> temperature;
        for(uint i = 0; i <= rcvdsplit.size(); i++)
        {
            if(regex_match(rcvdsplit[i],regex("79:([01]{5})")))
            {
                rcvdsplit[i] = regex_replace(rcvdsplit[i],regex("79:"),"");
                oxygen = bitset<5>(rcvdsplit[i]);
            } 
            else if(regex_match(rcvdsplit[i],regex("84:([01]{8})")))
            {
                rcvdsplit[i] = regex_replace(rcvdsplit[i],regex("84:"),"");
                temperature = bitset<8>(rcvdsplit[i]);
            }   
            else if(regex_match(rcvdsplit[i],regex("80:([01]{11})")))
            {
                rcvdsplit[i] = regex_replace(rcvdsplit[i],regex("80:"),"");
                pressure = bitset<11>(rcvdsplit[i]);
            }        
        }
        string outputmsg = "";
        double oxygenval = oxygen.to_ulong();
        double ostar = 20.0 * (oxygenval/16.0);
        for(int i = 0;i<20;i++)
        {
            if(i < ostar)
            {
                outputmsg += "*";
            }
            else
            {
                outputmsg += " ";
            }
        }
        outputmsg += "|";
        double temperatureval = temperature.to_ulong();
        double tstar = 20.0 * (temperatureval/128.0);
        for(int i = 0;i<20;i++)
        {
            if(i < tstar)
            {
                outputmsg += "*";
            }
            else
            {
                outputmsg += " ";
            }
        }
        outputmsg += "|";
        double pressureval = pressure.to_ulong();
        double pstar = 20.0 * (pressureval/1050.0);
        for(int i = 0;i<20;i++)
        {
            if(i < pstar)
            {
                outputmsg += "*";
            }
            else
            {
                outputmsg += " ";
            }
        }
        cout<<outputmsg<<endl;
    }
}
