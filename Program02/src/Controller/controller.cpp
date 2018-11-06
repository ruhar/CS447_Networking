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
#include "common.hpp"
#include <regex>
#include <locale>

using namespace std;
using namespace cs447;
int sequence = 0;
const int BUFFERSIZE = 64;

void cs447::Hello()
{
    cout<<"Hello captain, waiting for probe commands!"<<endl;
    cout<<"Usage:"<<endl;
    cout<<"The following quick command will send all headers and track proper cseq numbers."<<endl;
    cout<<"Quick Commands: setup, play, pause, teardown."<<endl;
    cout<<"For long command info, type: help"<<endl;
}
void cs447::Help()
{
    cout<<"\n"<<endl;
    cout<<"Setup Command:"<<endl;
    cout<<"setup rtsp://localhost rtsp/2.0 <crlf>"<<endl;
    cout<<"cseq:<sequencenumber> <crlf>"<<endl;
    cout<<"transport:udp;unicast;dest_addr=\":<receiver-port>\""<<endl;
    cout<<"sensor:<* or comma delimited t,p,o> <crlf>"<<endl<<endl;
    cout<<"Play Command:"<<endl;
    cout<<"play rtsp://localhost rtsp/2.0 <crlf>"<<endl;
    cout<<"cseq:<sequencenumber> <crlf>"<<endl;
    cout<<"sensor:<* or comma delimited t,p,o> <crlf>"<<endl<<endl;
    cout<<"Pause Command:"<<endl;
    cout<<"pause rtsp://localhost rtsp/2.0 <crlf>"<<endl;
    cout<<"cseq:<sequencenumber> <crlf>"<<endl<<endl;
    cout<<"Teardown Command:"<<endl;
    cout<<"teardown rtsp://localhost rtsp/2.0 <crlf>"<<endl;
    cout<<"cseq:<sequencenumber> <crlf>"<<endl<<endl;  
    cout<<"Sensor header is optional. All cseq numbers must be next value, except setup."<<endl;
}
void cs447::Goodbye()
{
    cout<<"Thank you for using the probe controller service!\n";    
}
void cs447::RTSPControlClient(std::string _ServerAddress, int _ServerPort, int _ReceiverPort)
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
    thread sendthread (RTSPSender,serverinfo,_ReceiverPort);
    thread rcvthread (RTSPReceiver,serverinfo);

    sendthread.join();
    rcvthread.detach();
    rcvthread.~thread();
    close(sck);
}
void cs447::RTSPSender(tcpargs _TCPArguments, int _ReceiverPort)
{
    int socket = _TCPArguments.socket;
    char node[NI_MAXHOST];
    getnameinfo((struct sockaddr*)&_TCPArguments.address, sizeof(_TCPArguments.address), node, sizeof(node),NULL, 0, NI_NAMEREQD);
    string hostname(node);
    vector<string> hostparts;
    StringSplit(hostname,hostparts,'.');
    hostname = hostparts[0];
    string input = "";
    string buffer = "";
    bool running = true;
    while(running)
    {
        input = "";
        getline(cin,input);
        buffer = input;
        if(regex_match(buffer,regex("( ){0,}setup( ){0,}(\\s){0,}",regex::icase)))
        {
            sequence = 0;
            buffer = "setup rtsp://" + hostname + " rtsp/2.0\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "cseq:" + to_string(sequence) + "\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "transport:UDP;unicast;dest_addr=\":" + to_string(_ReceiverPort) + "\"\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "sensor:*\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else if(regex_match(buffer,regex("( ){0,}play( ){0,}(\\s){0,}",regex::icase)))
        {
            buffer = "play rtsp://" + hostname + " rtsp/2.0\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "sensor:*\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "cseq:" + to_string(sequence) + "\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

        }
        else if(regex_match(buffer,regex("( ){0,}pause( ){0,}(\\s){0,}",regex::icase)))
        {
            buffer = "pause rtsp://" + hostname + " rtsp/2.0\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "cseq:" + to_string(sequence) + "\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else if(regex_match(buffer,regex("( ){0,}teardown( ){0,}(\\s){0,}",regex::icase)))
        {
            buffer = "teardown rtsp://" + hostname + " rtsp/2.0\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "cseq:" + to_string(sequence) + "\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
            this_thread::sleep_for(chrono::milliseconds(100));
            running = false;
        }
        else if(regex_match(buffer,regex("( ){0,}help( ){0,}(\\s){0,}",regex::icase)))
        {
            Help();
        }
        else
        {
            buffer += "\r\n";
            send(socket,buffer.c_str(),buffer.length(),0);
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
        if(regex_search(rcvdmsg,regex("RTSP/2.0 200 OK",regex::icase)))
        {
            sequence++;
        }
    }
}