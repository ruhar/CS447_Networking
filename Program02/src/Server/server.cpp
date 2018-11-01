#include <string>
#include "server.hpp"
#include "common.hpp"
#include "tcpargs.hpp"
#include "rtspheaders.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
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
bool oplaying = false;
bool tplaying = false;
bool pplaying = false;
int curroxygen = 0;
int currtemperature = 0;
int currpressure = 0;
int maxoxygen = 0;
int maxtemperature = 0;
int maxpressure = 0;

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
    // cout<<"Reading sensor data..."<<endl;
    thread o(ReadOxygenSensor,std::ref(oxygen),_OxygenFile,std::ref(maxoxygen));
    thread p(ReadPressureSensor,std::ref(pressure),_PressureFile,std::ref(maxpressure));
    thread t(ReadTemperatureSensor,std::ref(temperature),_TemperatureFile,std::ref(maxtemperature));
    thread oinc(SensorIncrement,std::ref(oplaying),std::ref(curroxygen),std::ref(maxoxygen));
    thread pinc(SensorIncrement,std::ref(pplaying),std::ref(currpressure),std::ref(maxpressure));
    thread tinc(SensorIncrement,std::ref(tplaying),std::ref(currtemperature),std::ref(maxtemperature));


    // bool &_Playing, int &_Counter, int &_Size
    // cout<<"Oxygen Count: "<< oxygen.size()<<endl;
    // cout<<"Pressure Count: "<< pressure.size()<<endl;
    // cout<<"Temperature Count: "<< temperature.size()<<endl;
    // o.join();
    // p.join();
    // t.join();
    // cout<<"Oxygen Count: "<< oxygen.size()<<endl;
    // cout<<"Pressure Count: "<< pressure.size()<<endl;
    // cout<<"Temperature Count: "<< temperature.size()<<endl;
    // cout<<"...Sensor reading complete."<<endl;
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
    vector<pthread_t> server_thread;

    while(true)
    {
        pthread_t newthread;
        server_thread.push_back(newthread);

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
            pthread_create(&server_thread.back(),NULL,RTSPServerHandler,(void *) &sckinfo);
        }
        cout<<"Thread count: "<< sizeof(server_thread)<<endl;
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
void *cs447::RTSPServerHandler(void *_sckinfo)
{
    bool listening = true;

    string hostname = GetHostName();    
    int sck = ((tcpargs *)_sckinfo)->socket;
    sockaddr_in socketaddress = ((tcpargs *)_sckinfo)->address;
    string msg = hostname + " Haddock's RTSP Service. Ready at " + GetCurrentTimeStamp();
    char buffer[BUFFERSIZE];
    memset(buffer, 0, BUFFERSIZE);
    rtspheaders connHeader;
    connHeader.CSeq = "0";
    connHeader.Date = GetCurrentTimeStamp();
    cout<<"Opening RTSP Control Thread for client IP: "<<inet_ntoa(socketaddress.sin_addr)<<endl;
    cout<<" Socket: "<<sck<<endl;
    RTSPSendResponse(sck,200,connHeader);
    int lostconn = 0;
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
            rtspheaders teardownHeader;
            teardownHeader.CSeq = "1";
            RTSPSendResponse(sck,200,teardownHeader);
            listening = false;
        }
        else if(regex_match(rcvdmsg,regex("setup rtsp:\\/\\/([0-9a-z]){1}([\\-0-9a-z]){0,}\\/ rtsp\\/2.0(\\s){1,}",regex::icase)) ||
                regex_match(rcvdmsg,regex("setup rtsp:\\/\\/([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\/ rtsp\\/2.0(\\s){1,}")))
        {
            lostconn = 0;
            rtspheaders setupHeader;
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
                    setupHeader.CSeq = trim(rcvdmsg);
                }
                if(regex_match(rcvdmsg,regex("(sensor:){1}( ){0,}((([otp*]){1},([otp]){1},([otp]){1})|(([otp*]){1},([otp]){1})|(([otp*]){1}))(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(sensor:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");
                    setupHeader.SetSensor(trim(rcvdmsg));
                }
                if(rcvdmsglength == 2 && rcvdmsg[0] == 13 && rcvdmsg[1] == 10)
                {
                    dataentry = false;
                }
            }while(dataentry);

            RTSPSendResponse(sck,200,setupHeader);
        }
        else if(regex_match(rcvdmsg,regex("play(\\s){0,}",regex::icase)))
        {
            cout<<"Playing to 127.0.0.1:8100:UDP"<<endl;
            lostconn = 0;
            rtspheaders setupHeader;
            int dataentry = 15;
            oplaying = true;
            do
            {
                // int rcvdmsglength;
                // memset(buffer, 0, BUFFERSIZE);
                // rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                // string rcvdmsg(buffer);
                // while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
                // {
                //     memset(buffer, 0, BUFFERSIZE);
                //     rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                //     rcvdmsg += buffer;
                // }
                // rcvdmsglength = rcvdmsg.length();
                //Send to UDP client
                int udpsck = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
                struct sockaddr_in saddress;
                saddress.sin_family = AF_INET; 
                saddress.sin_addr.s_addr = inet_addr("127.0.0.1");
                saddress.sin_port = htons(8100); 
                // char sndbuffer[BUFFERSIZE];
                // cout<<oxygen[dataentry].to_ulong()<<"|"<<oxygen[dataentry].to_string()<<endl;
                string sending = "79:" + oxygen[dataentry].to_string() + ";84:" + temperature[dataentry].to_string() + ";80:" + pressure[dataentry].to_string();// +"\r\n";
                cout<<sending<<endl;
                char sndbuffer[sending.length()];
                strcpy(sndbuffer,sending.c_str());
                sendto(udpsck, sndbuffer, sending.length(), 0,(struct sockaddr *) &saddress, sizeof(saddress));
                // if(rcvdmsglength == 2 && rcvdmsg[0] == 13 && rcvdmsg[1] == 10)
                // {
                //     dataentry = false;
                // }
                dataentry--;
            }while(dataentry >= 0);
            // cout<<"Oxygen Count: "<< oxygen.size()<<endl;
            // cout<<"Pressure Count: "<< pressure.size()<<endl;
            // cout<<"Temperature Count: "<< temperature.size()<<endl;

            RTSPSendResponse(sck,200,setupHeader);
        }
        else if(rcvdmsglength == 0)
        {
            lostconn++;
        }
        else
        {
            RTSPSendResponse(sck,200,connHeader);
        }
    }
    cout<<"Closing RTSP Control Thread for client IP: "<<inet_ntoa(socketaddress.sin_addr)<<endl;
    close(sck);
    return 0;
}
int cs447::RTSPSendResponse(int &_ClientSocket, int _ResponseCode, rtspheaders _Headers)
{
    string msg = "RTSP/2.0 ";
    switch (_ResponseCode)
    {
        case 200:
            msg += "200\r\n";
            msg += _Headers.PrintHeaders();
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
    ifstream sensor(_FileName, ios::binary|ios::in);
    char c;
    string bitstring = "00000";
    int count = 4;
    // int stop = 0;
    while(sensor.get(c))
    {
        unsigned char uc = static_cast<unsigned char>(c);
        // cout<<c<<"|"<<uc<<endl;
        for(int i = 0; i<8;i++)
        {
            int remainder = uc % 2;
            // cout<<remainder;
            bitstring[count] = to_string(remainder)[0];
            count--;
            if(count < 0)
            {
                std::bitset<5> newData (bitstring);
                if(newData.to_ulong() <= OXYGENMAX)
                {
                    _SensorData.push_back(newData);
                    // cout<<endl<<newData.to_string()<<endl;
                    _MaxReadings++;
                }
                else
                {
                    newData = bitset<5>("10000");
                    _SensorData.push_back(newData);
                    _MaxReadings++;
                }
                bitstring = "00000";
                count = 4;
            }
            uc = (uc - remainder) / 2;
        }
        // cout<<endl;
        // stop++;
    }
}
void cs447::ReadPressureSensor(vector<bitset<11>> &_SensorData, string _FileName, int &_MaxReadings)
{
    ifstream sensor(_FileName, ios::binary|ios::in);
    char c;
    string bitstring = "00000000000";
    int count = 10;
    while(sensor.get(c))
    {
        unsigned char uc = static_cast<unsigned char>(c);
        for(int i = 0; i<8;i++)
        {
            int remainder = uc % 2;
            bitstring[count] = to_string(remainder)[0];
            count--;
            if(count < 0)
            {
                bitset<11> newData (bitstring);
                //if(newData.to_ulong() <= PRESSUREMAX)
                {
                    _SensorData.push_back(newData);
                }
                bitstring = "00000000000";
                count = 10;
            }
            uc = (uc - remainder) / 2;
        }
    }
}
void cs447::ReadTemperatureSensor(vector<bitset<8>> &_SensorData, string _FileName, int &_MaxReadings)
{
    ifstream sensor(_FileName, ios::binary|ios::in);
    char c;
    string bitstring = "00000000";
    int count = 7;
    while(sensor.get(c))
    {
        unsigned char uc = static_cast<unsigned char>(c);
        for(int i = 0; i<8;i++)
        {
            int remainder = uc % 2;
            bitstring[count] = to_string(remainder)[0];
            count--;
            if(count < 0)
            {
                bitset<8> newData (bitstring);
                newData.set(7,0);//Ignore most significant bit per Dr. Gamage
                //if(newData.to_ulong() <= TEMPERATUREMAX)
                {
                    _SensorData.push_back(newData);
                }
                bitstring = "00000000";
                count = 7;
            }
            uc = (uc - remainder) / 2;
        }
    }
}
void cs447::RTSPPlay(tcpargs _SocketInfo)
{
    int udpsck = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in saddress = _SocketInfo.address;
    saddress.sin_family = AF_INET; 
    saddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    saddress.sin_port = htons(8100); 
    // char sndbuffer[BUFFERSIZE];
    // cout<<oxygen[dataentry].to_ulong()<<"|"<<oxygen[dataentry].to_string()<<endl;
    // string sending = "79:" + oxygen[dataentry].to_string() + ";84:" + temperature[dataentry].to_string() + ";80:" + pressure[dataentry].to_string();// +"\r\n";
    // cout<<sending<<endl;
    // char sndbuffer[sending.length()];
    // strcpy(sndbuffer,sending.c_str());
    // sendto(udpsck, sndbuffer, sending.length(), 0,(struct sockaddr *) &saddress, sizeof(saddress));
}
void cs447::SensorIncrement(bool &_Playing, int &_Counter, int &_Size)
{
    std::chrono::seconds duration(3);
    while(true)
    {
        if(_Playing && _Counter < _Size)
        {
            cout<<"Counter: "<<_Counter<<endl;
            _Counter++;
        }
        else
        {
            _Counter = 0;
        }
        this_thread::sleep_for(duration);
    }
}