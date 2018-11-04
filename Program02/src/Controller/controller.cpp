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
#include <netdb.h>
#include <netinet/tcp.h>

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
    // int yes = 1;
    // int result = setsockopt(sck, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int)); 
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
    int yes = 1;
    int no = 0;
    // int result = setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, (char *) &yes, sizeof(int)); 

    // struct sockaddr_in saddress = _TCPArguments.address;
    char node[NI_MAXHOST];
    getnameinfo((struct sockaddr*)&_TCPArguments.address, sizeof(_TCPArguments.address), node, sizeof(node),NULL, 0, NI_NAMEREQD);
    string hostname(node);
    // cout<<"Hostname:"<<hostname<<endl;

    string input = "";
    while(input != "teardown\r\n")
    {
        // int length;
        input = "";
        getline(cin,input);
        if(input == "setup")
        {
            cout<<"Setup"<<endl;
            input = "setup rtsp://localhost rtsp/2.0\r\n";
            cout<<input.length()<<"|"<<input;
            send(socket,input.c_str(),input.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));
            input = "cseq:10\r\n";
            cout<<input.length()<<"|"<<input;
            send(socket,input.c_str(),input.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));
            // input = "transport:UDP;unicast;dest_addr=\":" + to_string(ntohs(_TCPArguments.address.sin_port)) + "\"\r\n";
            // cout<<input.length()<<"|"<<input;
            // send(socket,input.c_str(),input.length(),0);

            input = "sensor:*\r\n";
            cout<<input.length()<<"|"<<input;
            send(socket,input.c_str(),input.length(),0);
            setsockopt(socket,SOL_TCP, TCP_CORK, &no, sizeof(no));
            this_thread::sleep_for(chrono::milliseconds(100));

            setsockopt(socket,SOL_TCP, TCP_CORK, &yes, sizeof(yes));
            input = "\r\n";
            cout<<input.length()<<"|"<<input;
            send(socket,input.c_str(),input.length(),0);
            setsockopt(socket,SOL_TCP, TCP_CORK, &no, sizeof(no));
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else
        {
            input += "\r\n";
            // length = input.length();
            // char buffer[length + 1];
            // strcpy(buffer,input.c_str());
            cout<<input.length()<<"|"<<input;           
            send(socket,input.c_str(),input.length(),0);
        }
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