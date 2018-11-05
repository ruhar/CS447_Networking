#include <string>
#include "server.hpp"
#include "common.hpp"
#include "tcpargs.hpp"
#include "rtspheaders.hpp"
#include "SensorControl.hpp"
#include "SensorControlClient.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>

#include <unistd.h>
#include <vector>
#include <netdb.h>
#include <regex>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <iomanip>
#include <thread>
#include <math.h>
#include <bitset>
#include <chrono>

using namespace std;
using namespace cs447;

static int sckaccept;
const int BUFFERSIZE = 64;
const uint OXYGENMAX = 16;
const uint PRESSUREMAX = 1050;
const uint TEMPERATUREMAX = 255;
enum {DATE = 0,FROM,TO,SUBJECT,MESSAGE};
enum {GET=0,HOST,COUNT};
enum {RESPONSE=0,SERVER,LASTMODIFIED,RETRIEVECOUNT,CONTENTTYPE,MESSAGECOUNT};
vector<bitset<5>> oxygen;
vector<bitset<11>> pressure;
vector<bitset<8>> temperature;
int curroxygen = 0;
int currtemperature = 0;
int currpressure = 0;
int maxoxygen = 0;
int maxtemperature = 0;
int maxpressure = 0;
SensorControl sControl;

void cs447::Hello()
{
    cout<<"Welcome to the electronic age Captain!\n";
}
void cs447::Goodbye()
{
    cout<<"Thank you for using Dr. Calculus's mail services!\n";    
}

void cs447::RTSPServer(int _Port, string _OxygenFile, string _TemperatureFile, string _PressureFile)
{
    thread o(ReadOxygenSensor,std::ref(oxygen),_OxygenFile,std::ref(maxoxygen));
    thread p(ReadPressureSensor,std::ref(pressure),_PressureFile,std::ref(maxpressure));
    thread t(ReadTemperatureSensor,std::ref(temperature),_TemperatureFile,std::ref(maxtemperature));
    thread oinc(SensorIncrement,std::ref(sControl.oplaying),std::ref(curroxygen),std::ref(maxoxygen));
    thread pinc(SensorIncrement,std::ref(sControl.pplaying),std::ref(currpressure),std::ref(maxpressure));
    thread tinc(SensorIncrement,std::ref(sControl.tplaying),std::ref(currtemperature),std::ref(maxtemperature));

    struct sockaddr_in saddress;
    saddress.sin_family = AF_INET;
    saddress.sin_port = htons(_Port);
    saddress.sin_addr.s_addr = htonl(INADDR_ANY);  
    
    int sck = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sck < 0)
    {
        throw runtime_error("Unable to start socket.");
    }

    int sckbind = bind(sck,(struct sockaddr *) &saddress,sizeof(saddress));
    if(sckbind < 0)
    {
        throw runtime_error("Unable to bind to port " + to_string(_Port));
    }

    int scklisten = listen(sck,5);
    if(scklisten < 0)
    {
        throw runtime_error("Unable to listien on port " + to_string(_Port));
    }
    cout<<"Waiting for TCP Connections on port "<<_Port<<"\n"<<endl;
    struct sockaddr_in caddress;
    // vector<pthread_t> server_thread;
    vector<std::thread> server_thread;
    while(true)
    {
        // pthread_t newthread;
        // server_thread.push_back(newthread);

        socklen_t caddr_length = sizeof(caddress);
        sckaccept = accept(sck,(struct sockaddr *) &caddress,&caddr_length);

        if(sckaccept < 0)
        {
            cout<<"Client connection refused from: "<<inet_ntoa(caddress.sin_addr)<<endl;
        }
        else
        {
            tcpargs sckinfo;
            sckinfo.address = caddress;
            sckinfo.socket = sckaccept;
            server_thread.push_back(thread(RTSPServerHandler,sckinfo));
        }
    }   
}
void cs447::SMTPServer(int _Port)
{

    struct sockaddr_in saddress;
    saddress.sin_family = AF_INET;
    saddress.sin_port = htons(_Port);
    saddress.sin_addr.s_addr = htonl(INADDR_ANY);  
    
    int sck = socket(PF_INET,SOCK_STREAM,0);
    if(sck < 0)
    {
        throw runtime_error("Unable to start socket.");
    }

    int sckbind = bind(sck,(struct sockaddr *) &saddress,sizeof(saddress));
    if(sckbind < 0)
    {
        throw runtime_error("Unable to bind to port " + to_string(_Port));
    }

    int scklisten = listen(sck,5);
    if(scklisten < 0)
    {
        throw runtime_error("Unable to listien on port " + to_string(_Port));
    }
    cout<<"Waiting for TCP Connections on port "<<_Port<<"\n"<<endl;
    while(true)
    {
        pthread_t server_thread;
        struct sockaddr_in caddress;
        socklen_t caddr_length = sizeof(caddress);
        sckaccept = accept(sck,(struct sockaddr *) &caddress,&caddr_length);
        if(sckaccept < 0)
        {
            cout<<"Client connection refused from: "<<inet_ntoa(caddress.sin_addr)<<endl;
        }
        else
        {
            tcpargs sckinfo;
            sckinfo.address = caddress;
            sckinfo.socket = sckaccept;
            pthread_create(&server_thread,NULL,&SMTPServerHandler,(void *) &sckinfo);
        }
    }
}
void *cs447::SMTPServerHandler(void *_sckinfo)
{
    bool listening = true;
    bool helosent = false;
    bool mailfrom = false;
    bool rcptto = false;
    string hostname = GetHostName();    
    tcpargs *sckinfo = (tcpargs *)_sckinfo;
    string msg = hostname + " Haddock's SMTP Service. Ready at " + GetCurrentTimeStamp();
    char buffer[BUFFERSIZE];
    memset(buffer, 0, BUFFERSIZE);
    SMTPSendResponse(sckinfo->socket,220, msg);
    cout<<"Opening SMTP Thread for client IP: "<<inet_ntoa(sckinfo->address.sin_addr)<<endl;
    while(listening)
    {
        int rcvdmsglength;
        rcvdmsglength = recv(sckinfo->socket,buffer,BUFFERSIZE,0);
        string rcvdmsg(buffer);
        while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
        {
            memset(buffer, 0, BUFFERSIZE);
            rcvdmsglength = recv(sckinfo->socket,buffer,BUFFERSIZE,0);
            rcvdmsg += buffer;
        }
        memset(buffer, 0, BUFFERSIZE);
        if(regex_match(rcvdmsg,regex("^(quit)(\\s){0,}",regex::icase)))
        {
            SMTPSendResponse(sckinfo->socket,221,"Thank you for using Dr. C's Mail Services!");
            listening = false;
        }
        else if(regex_match(rcvdmsg,regex("^(helo)(\\s){0,}",regex::icase)))
        {
            helosent = SMTPHelo(sckinfo->socket,sckinfo->address);
        }
        else if(regex_match(rcvdmsg,regex("^(mail)(\\s){1,}(.){0,}(\\s){0,}",regex::icase)))
        {
            if(helosent)
            {
                if(regex_match(rcvdmsg,regex("^(mail)(\\s){1,}(from:)(.){0,}(\\s){0,}",regex::icase)))
                {
                    if(regex_match(rcvdmsg,regex("^(mail)(\\s){1,}(from:)(\\s){0,}(<){0,1}[\\w._%+-]+(@cs447f18.edu)(>){0,1}(\\s){0,}",regex::icase)))
                    {
                        int firstcolon = rcvdmsg.find_first_of(':');
                        string email = rcvdmsg.substr(firstcolon + 1);
                        email = trim(email, 10);
                        email = trim(email);
                        email = ltrim(email,'<');
                        email = rtrim(email,'>');
                        SMTPSendResponse(sckinfo->socket,250,"Email from accepted.");
                        mailfrom = true;
                    }
                    else
                    {
                        SMTPSendResponse(sckinfo->socket,501,"Invalid email address.");
                    }
                }
                else
                {
                    SMTPSendResponse(sckinfo->socket,501,"Invalid Argument - must be \"mail from:\"");
                }
            }
            else
            {
                SMTPSendResponse(sckinfo->socket,503,"Send HELO first");
            }

        }
        else if(regex_match(rcvdmsg,regex("^(rcpt)(\\s){0,}(.){0,}(\\s){0,}",regex::icase)))
        {
            if(mailfrom)
            {
                if(regex_match(rcvdmsg,regex("(rcpt)(\\s){1,}(to:)(\\s){0,}(.){0,}(\\s){0,}",regex::icase)))
                {
                    if(regex_match(rcvdmsg,regex("^(rcpt)(\\s){1,}(to:)(\\s){0,}(<){0,1}[\\w._%+-]+(@cs447f18.edu)(>){0,1}(\\s){0,}",regex::icase)))
                    {
                        int firstcolon = rcvdmsg.find_first_of(':');
                        string email = rcvdmsg.substr(firstcolon + 1);
                        email = trim(email, 10);
                        email = trim(email);
                        email = ltrim(email,'<');
                        email = rtrim(email,'>');
                        SMTPSendResponse(sckinfo->socket,250,"Email recipient accepted.");
                        rcptto = true;
                    }
                    else
                    {
                        SMTPSendResponse(sckinfo->socket,501,"Invalid email address.");
                    }
                }
                else
                {
                    SMTPSendResponse(sckinfo->socket,501,"Invalid Argument - must be \"rcpt to:\"");
                }
            }
            else
            {
                SMTPSendResponse(sckinfo->socket,503,"Send MAIL FROM: first");
            }
        }
        else if(regex_match(rcvdmsg,regex("^(data)(\\s){0,}",regex::icase)))
        {
            if(rcptto)
            {
                SMTPSendResponse(sckinfo->socket,354);
                bool dataentry = true;
                memset(buffer,0,BUFFERSIZE);
                rcvdmsg = "";
                while(dataentry)
                {
                    rcvdmsglength = recv(sckinfo->socket,buffer,BUFFERSIZE,0);
                    if(regex_match(buffer,regex("^(\\.\\r\\n)$",regex::icase))||
                    regex_match(buffer,regex("^(\\.\\n)$",regex::icase)))
                    {
                        dataentry = false;
                    } 
                    else
                    {
                        rcvdmsg += buffer;
                    }
                    while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
                    {
                        memset(buffer, 0, BUFFERSIZE);
                        rcvdmsglength = recv(sckinfo->socket,buffer,BUFFERSIZE,0);
                        rcvdmsg += buffer;
                    }
                    memset(buffer, 0, BUFFERSIZE);
                }
                SMTPSendResponse(sckinfo->socket,250,"Data entry complete");
            }
            else
            {
                SMTPSendResponse(sckinfo->socket,503,"Send RCPT TO: first");
            }
        }
        else
        {
            SMTPSendResponse(sckinfo->socket,500);
        }
    }
    cout<<"Closing SMTP Thread for client IP: "<<inet_ntoa(sckinfo->address.sin_addr)<<endl;
    close(sckinfo->socket);
    return 0;
}
int cs447::SMTPSendResponse(int &_ClientSocket, int _ResponseCode, string _Message)
{
    string msg = "";
    switch (_ResponseCode)
    {
        case 220:
            msg = "220 Hello: " + _Message + "\n";
            break;
        case 221:
            msg = "221 Goodbye: " + _Message + "\n";
            break;
        case 250:
            msg = "250 OK:" + _Message + "\n";
            break;
        case 354:
            msg = "354 Start mail input, end with <CRLF>.<CRLF>\n";
            break;
        case 500:
            msg = "500 Command syntax error: " +_Message + "\n";
            break;
        case 501:
            msg = "501 Argument syntax error: " + _Message + "\n";
            break;
        case 502:
            msg = "502 Command not implemented: " + _Message + "\n";
            break;
        case 503:
            msg = "503 Bad sequence of commands: " +_Message + "\n";
            break;  
        case 554:
            msg = "554 Transaction failed:";
            break;
        default:
            msg += _ResponseCode + " " + _Message + "\n";
            break;
    }
    if(msg.length() > 0)
    {
        // char msgSend[msg.length()];
        // strcpy(msgSend,msg.c_str());
        // return send(_ClientSocket,msgSend,sizeof(msgSend),0);
        char msgSend[msg.length()];
        strcpy(msgSend,msg.c_str());
        int retval = 90210;
        try
        {
            retval = send(_ClientSocket,msgSend,sizeof(msgSend),0);
        }
        catch(const std::exception& e)
        {
            cout << e.what() << '\n';
        }        
        return retval;
    }
    else
    {
        return -1;
    }
}
bool cs447::SMTPHelo(int &_ClientSocket, sockaddr_in _ClientAddress)
{
    string clientinfo(inet_ntoa(_ClientAddress.sin_addr));
    int response = SMTPSendResponse(_ClientSocket,250,"Hello [" + clientinfo + "]");
    if(response < 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void cs447::RTSPServerHandler(tcpargs _sckinfo)
{
    bool listening = true;
    RTSPHeaders headers = RTSPHeaders();
    vector<std::thread> player;


    string hostname = GetHostName();    
    // int sck = ((tcpargs *)_sckinfo)->socket;
    // sockaddr_in socketaddress = ((tcpargs *)_sckinfo)->address;
    int sck = _sckinfo.socket;
    sockaddr_in socketaddress = _sckinfo.address;

    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getsockname(sck, (struct sockaddr *)&addr, &addr_size);
    string serverIP = inet_ntoa(addr.sin_addr);
    string serverPort = to_string(ntohs(addr.sin_port));

    sControl.AddClient(sck);
    // string msg = hostname + " Haddock's RTSP Service. Ready at " + GetCurrentTimeStamp();
    char buffer[BUFFERSIZE];
    memset(buffer, 0, BUFFERSIZE);
    headers.CSeq = 0;
    headers.Headers[(int)HEADER::CONNECTION].Date = GetCurrentTimeStamp();
    cout<<"Opening RTSP Control Thread for client IP: "<<inet_ntoa(socketaddress.sin_addr)<<endl;
    cout<<" Socket: "<<sck<<endl;
    RTSPSendResponse(sck,200,headers,HEADER::CONNECTION);
    int lostconn = 0;
    vector<string> msg;
    vector<string> setupmsg;
    while(listening && lostconn < 10)
    {
        int rcvdmsglength;
        try
        {
            rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
        }
        catch(exception er)
        {
            cout<<er.what();
        }
        string rcvdmsg(buffer);
        while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
        {
            memset(buffer, 0, BUFFERSIZE);
            rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
            rcvdmsg += buffer;
        }
        memset(buffer, 0, BUFFERSIZE);

        if(regex_match(rcvdmsg,regex("^(teardown)(\\s){0,}",regex::icase)))
        {
            lostconn = 0;
            RTSPSendResponse(sck,200,headers,HEADER::TEARDOWN);
            int udpsck = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
            struct sockaddr_in saddress;
            saddress.sin_family = AF_INET; 
            saddress.sin_addr.s_addr = inet_addr("127.0.0.1");
            saddress.sin_port = htons(8100); 
            string sending = "teardown";
            cout<<sending<<endl;
            char sndbuffer[sending.length()];
            strcpy(sndbuffer,sending.c_str());
            sendto(udpsck, sndbuffer, sending.length(), 0,(struct sockaddr *) &saddress, sizeof(saddress));
            listening = false;
        }
        else if(regex_match(rcvdmsg,regex("setup rtsp:\\/\\/([0-9a-z]){1}([\\-0-9a-z]){0,}(\\/){0,1} rtsp\\/2.0(\\s){0,}",regex::icase)) ||
                regex_match(rcvdmsg,regex("setup rtsp:\\/\\/([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})(\\/){0,1} rtsp\\/2.0(\\s){0,}")))
        {
            lostconn = 0;
            bool dataentry = true;
            do
            {
                int rcvdmsglength;
                memset(buffer, 0, BUFFERSIZE);
                rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                string rcvdmsg(buffer);
                while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
                {
                    memset(buffer, 0, BUFFERSIZE);
                    rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                    rcvdmsg += buffer;
                }
                rcvdmsglength = rcvdmsg.length();
                if(regex_match(rcvdmsg,regex("^(cseq:){1}( ){0,}[0-9]{1,5}(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(cseq:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");                    
                    headers.CSeq = stoi(trim(rcvdmsg));
                }
                if(regex_match(rcvdmsg,regex("(sensor:){1}( ){0,}((([otp*]){1},([otp]){1},([otp]){1})|(([otp*]){1},([otp]){1})|(([otp*]){1}))(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(sensor:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");
                    headers.Headers[(int)HEADER::SETUP].SetSensor(trim(rcvdmsg));
                }
                if(regex_match(rcvdmsg,regex("(transport:){1}( ){0,}(UDP;){1}(unicast;){1}(dest_addr=\":){1}[0-9]{1,5}(\"){1}(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(transport:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");
                    vector<string> transport;
                    StringSplit(rcvdmsg,transport,';');
                    headers.Headers[(int)HEADER::SETUP].TransportInfo.Protocol = transport[0];
                    headers.Headers[(int)HEADER::SETUP].TransportInfo.Transmission = transport[1]; 
                    transport[2] = regex_replace(transport[2],regex("(dest_addr=\":)",regex::icase),"");
                    transport[2] = regex_replace(transport[2],regex("(\")",regex::icase),"");
                    headers.Headers[(int)HEADER::SETUP].TransportInfo.DestAddress = inet_ntoa(socketaddress.sin_addr);
                    headers.Headers[(int)HEADER::SETUP].TransportInfo.DestPort = transport[2];
                    headers.Headers[(int)HEADER::SETUP].TransportInfo.SrcAddress = serverIP;
                    headers.Headers[(int)HEADER::SETUP].TransportInfo.SrcPort = serverPort;
                }
                if((rcvdmsglength == 2 && rcvdmsg[0] == 13 && rcvdmsg[1] == 10))
                {
                    dataentry = false;
                }
            }while(dataentry);
            RTSPSendResponse(sck,200,headers,HEADER::SETUP);
            string IPAddress = headers.Headers[(int)HEADER::SETUP].TransportInfo.DestAddress;
            int port = stoi(headers.Headers[(int)HEADER::SETUP].TransportInfo.DestPort);
            cout<<"1"<<endl;
            cout<<"Port: "<< port << endl;
            cout<<"IPAddress: "<< IPAddress << endl;
            cout<<"Socket: " << sck << endl;

            player.push_back(thread(RTSPPlay,sck,IPAddress,port));
            cout<<"2"<<endl;
        }
        else if(regex_match(rcvdmsg,regex("play rtsp:\\/\\/([0-9a-z]){1}([\\-0-9a-z]){0,}(\\/){0,1} rtsp\\/2.0(\\s){1,}",regex::icase)) ||
                regex_match(rcvdmsg,regex("play rtsp:\\/\\/([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})(\\/){0,1} rtsp\\/2.0(\\s){1,}")))
        {
            // cout<<"O:"<<maxoxygen<<"T:"<<maxtemperature<<"P:"<<maxpressure<<endl;
            lostconn = 0;
            bool dataentry = true;
            do
            {
                int rcvdmsglength;
                memset(buffer, 0, BUFFERSIZE);
                rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                string rcvdmsg(buffer);
                while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
                {
                    memset(buffer, 0, BUFFERSIZE);
                    rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                    rcvdmsg += buffer;
                }
                rcvdmsglength = rcvdmsg.length();
                if(regex_match(rcvdmsg,regex("^(cseq:){1}( ){0,}[0-9]{1,5}(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(cseq:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");                    
                    headers.CSeq = stoi(trim(rcvdmsg));
                    // sequence = stoi(playHeader.CSeq);
                }
                if(regex_match(rcvdmsg,regex("(sensor:){1}( ){0,}((([otp*]){1},([otp]){1},([otp]){1})|(([otp*]){1},([otp]){1})|(([otp*]){1}))(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(sensor:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");
                    headers.Headers[(int)HEADER::PLAY].SetSensor(trim(rcvdmsg));
                }
                if(rcvdmsglength == 2 && rcvdmsg[0] == 13 && rcvdmsg[1] == 10)
                {
                    dataentry = false;
                }
            }while(dataentry);

            RTSPSendResponse(sck,200,headers,HEADER::PLAY);
            // sequence++;

            sControl.SetPlaying(sck,headers.Headers[(int)HEADER::PLAY].Sensors[(int)SENSOR::OXYGEN],headers.Headers[(int)HEADER::PLAY].Sensors[(int)SENSOR::TEMPERATURE],headers.Headers[(int)HEADER::PLAY].Sensors[(int)SENSOR::PRESSURE]);
            cout<<"Playing to 127.0.0.1:8100:UDP"<<endl;
            lostconn = 0;

            int datacounter = 0;
            do
            {
                //Send to UDP client
                int udpsck = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
                struct sockaddr_in saddress;
                saddress.sin_family = AF_INET; 
                saddress.sin_addr.s_addr = inet_addr("127.0.0.1");
                saddress.sin_port = htons(8100); 
                string sending = "79:" + oxygen[datacounter].to_string() + ";84:" + temperature[datacounter].to_string() + ";80:" + pressure[datacounter].to_string();// +"\r\n";
                cout<<sending<<endl;
                char sndbuffer[sending.length()];
                strcpy(sndbuffer,sending.c_str());
                sendto(udpsck, sndbuffer, sending.length(), 0,(struct sockaddr *) &saddress, sizeof(saddress));
                datacounter++;
            }while(datacounter < 15);
        }
        else if(regex_match(rcvdmsg,regex("pause rtsp:\\/\\/([0-9a-z]){1}([\\-0-9a-z]){0,}(\\/){0,1} rtsp\\/2.0(\\s){1,}",regex::icase)) ||
                regex_match(rcvdmsg,regex("pause rtsp:\\/\\/([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})(\\/){0,1} rtsp\\/2.0(\\s){1,}")))
        {
            lostconn = 0;
            bool dataentry = true;
            do
            {
                int rcvdmsglength;
                memset(buffer, 0, BUFFERSIZE);
                rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                string rcvdmsg(buffer);
                while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
                {
                    memset(buffer, 0, BUFFERSIZE);
                    rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                    rcvdmsg += buffer;
                }
                rcvdmsglength = rcvdmsg.length();
                if(regex_match(rcvdmsg,regex("^(cseq:){1}( ){0,}[0-9]{1,5}(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(cseq:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");                    
                    headers.CSeq = stoi(trim(rcvdmsg));
                    // sequence = stoi(pauseHeader.CSeq);
                }
                if(rcvdmsglength == 2 && rcvdmsg[0] == 13 && rcvdmsg[1] == 10)
                {
                    dataentry = false;
                }
            }while(dataentry);

            RTSPSendResponse(sck,200,headers,HEADER::PAUSE);
            // sequence++;
            sControl.SetPlaying(sck,false,false,false);
        }
        else if(rcvdmsglength == 0)
        {
            lostconn++;
        }
        else
        {
            string sending = rcvdmsg;
            // string sending = "79:" + oxygen[headers.CSeq].to_string() + ";84:" + temperature[headers.CSeq].to_string() + ";80:" + pressure[headers.CSeq].to_string();// +"\r\n";
            cout<<sending<<endl;
            headers.CSeq++;
            RTSPSendResponse(sck,200,headers,HEADER::CONNECTION);
        }
    }
    cout<<"Closing RTSP Control Thread for client IP: "<<inet_ntoa(socketaddress.sin_addr)<<endl;
    sControl.DisconnectClient(sck);
    for(int i = 0; i < player.size(); i++)
    {
        player[i].join();
    }
    close(sck);
}
int cs447::RTSPSendResponse(int &_ClientSocket, int _ResponseCode, RTSPHeaders &_Headers, HEADER _Header)
{
    string msg = "RTSP/2.0 ";
    switch (_ResponseCode)
    {
        case 200:
            msg += "200\r\n";
            msg += _Headers.PrintHeader(_Header);
            break;
        default:
            msg += _ResponseCode + "\r\n";
            break;
    }
    if(msg.length() > 0)
    {
        char msgSend[msg.length()];
        strcpy(msgSend,msg.c_str());
        int retval = 90210;
        try
        {
            retval = send(_ClientSocket,msgSend,sizeof(msgSend),0);
        }
        catch(const std::exception& e)
        {
            cout << e.what() << '\n';
        }
        
        return retval;
    }
    else
    {
        return -1;
    }
}
void cs447::ReadOxygenSensor(vector<bitset<5>> &_SensorData, string _FileName, int &_MaxReadings)
{
    const int BITS = 5;
    const string ZEROBITS = "00000";
    const string MAXBITS = "10000";
    const int MAXBITSVALUE = 16;

    ifstream sensor(_FileName, ios::binary|ios::in);
    char c;
    int count = 0;
    int bitcount = 0;
    bitset<BITS> item(ZEROBITS);
    while(sensor.get(c))
    {
        unsigned char uc = static_cast<unsigned char>(c);
        for(int i = 0;i < 8;i++)
        {
            int bit = uc % 2;
            uc = (uc - bit)/2;
            int setbit = ((count * 8) + i) % BITS;
            item.set(setbit,bit);
            bitcount++;
            if(bitcount == BITS)
            {
                if(item.to_ulong() > MAXBITSVALUE)
                {
                    item = bitset<BITS>(MAXBITS);
                }
                _SensorData.push_back(item);
                item = bitset<BITS>(ZEROBITS);
                bitcount = 0;
                _MaxReadings++;
            }
        }       
        count++;
    }
}
void cs447::ReadPressureSensor(vector<bitset<11>> &_SensorData, string _FileName, int &_MaxReadings)
{
    const int BITS = 11;
    const string ZEROBITS = "00000000000";
    const string MAXBITS = "10000011010";
    const int MAXBITSVALUE = 1050;

    ifstream sensor(_FileName, ios::binary|ios::in);
    char c;
    int count = 0;
    int bitcount = 0;
    bitset<BITS> item(ZEROBITS);
    while(sensor.get(c))
    {
        unsigned char uc = static_cast<unsigned char>(c);
        for(int i = 0;i < 8;i++)
        {
            int bit = uc % 2;
            uc = (uc - bit)/2;
            int setbit = ((count * 8) + i) % BITS;
            item.set(setbit,bit);
            bitcount++;
            if(bitcount == BITS)
            {
                if(item.to_ulong() > MAXBITSVALUE)
                {
                    item = bitset<BITS>(MAXBITS);
                }
                _SensorData.push_back(item);
                item = bitset<BITS>(ZEROBITS);
                bitcount = 0;
                _MaxReadings++;
            }
        }       
        count++;
    }
}
void cs447::ReadTemperatureSensor(vector<bitset<8>> &_SensorData, string _FileName, int &_MaxReadings)
{
    const int BITS = 8;
    const string ZEROBITS = "00000000";
    const string MAXBITS = "01111111";
    const int MAXBITSVALUE = 127;

    ifstream sensor(_FileName, ios::binary|ios::in);
    char c;
    int count = 0;
    int bitcount = 0;
    bitset<BITS> item(ZEROBITS);
    while(sensor.get(c))
    {
        unsigned char uc = static_cast<unsigned char>(c);
        for(int i = 0;i < 8;i++)
        {
            int bit = uc % 2;
            uc = (uc - bit)/2;
            int setbit = ((count * 8) + i) % BITS;
            //Ignoring left most bit per Dr. Gamage email
            if(setbit == 7)
            {
                item.set(setbit,0);
            }
            else
            {
                item.set(setbit,bit);
            }
            bitcount++;
            if(bitcount == BITS)
            {
                if(item.to_ulong() > MAXBITSVALUE)
                {
                    item = bitset<BITS>(MAXBITS);
                }
                _SensorData.push_back(item);
                item = bitset<BITS>(ZEROBITS);
                bitcount = 0;
                _MaxReadings++;
            }
        }       
        count++;
    }
}
void cs447::RTSPPlay(int _Socket, std::string _ReceiverIP, int _ReceiverPort)
{
    int udpsck = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in saddress;
    saddress.sin_family = AF_INET; 
    saddress.sin_addr.s_addr = inet_addr(_ReceiverIP.c_str());
    saddress.sin_port = htons(_ReceiverPort); 
    string msg = "";
    SensorControlClient *_Client = sControl.GetClient(_Socket);
    while(!_Client->killthread)
    {
        msg = "";
        if(_Client->oplaying)
        {
            msg += "79:" + oxygen[curroxygen].to_string() + ";";
        }
        else
        {
            msg += "79:00000;";
        }
        if(_Client->tplaying)
        {
            msg += "84:" + temperature[currtemperature].to_string() + ";";
        }
        else
        {
            msg += "84:00000000;";
        }
        if(_Client->pplaying)
        {
            msg += "80:" + pressure[currpressure].to_string();
        }
        else
        {
            msg += "80:00000000000"; 
        }
        if(_Client->pplaying || _Client->tplaying || _Client->oplaying)
        {
            sendto(udpsck, msg.c_str(), msg.length(), 0,(struct sockaddr *) &saddress, sizeof(saddress));
        }
        this_thread::sleep_for(chrono::seconds(3));
    }
    cout<<"thread closing"<<endl;
}
void cs447::SensorIncrement(bool &_Playing, int &_Counter, int &_Size)
{
    std::chrono::seconds duration(3);
    while(true)
    {
        if(_Playing)
        {
            cout<<"Counter: "<<_Counter<<endl;
            _Counter++;
        }
        if(_Counter >= _Size)
        {
            _Counter = 0;
        }
        this_thread::sleep_for(duration);
    }
}