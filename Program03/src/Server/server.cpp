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
#include <openssl/ssl.h>
#include <openssl/err.h>

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
    cout<<"Captain Haddock's streaming sensor probe!\n";
}
void cs447::Goodbye()
{
    cout<<"Thank you for using sensor probing services!\n";    
}

void ShowCerts(SSL* ssl)
{   X509 *cert;
    char *line;
 
    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else
        printf("No certificates.\n");
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
    cout<<"Waiting for TCP Connections at "<< inet_ntoa(saddress.sin_addr) <<" on port "<<_Port<<"\n"<<endl;
    struct sockaddr_in caddress;
    vector<std::thread> server_thread;
    while(true)
    {
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
            thread(RTSPServerHandler,sckinfo).detach();
        }
    }   
}
void cs447::RTSPServerHandler(tcpargs _sckinfo)
{
    int clntsck = _sckinfo.socket;
    sockaddr_in socketaddress = _sckinfo.address;
    //SSL Setup
    const SSL_METHOD *method;
    method = TLS_server_method();
    SSL_CTX *ctx;
    //ssl initialize
    SSL_load_error_strings();
    ERR_print_errors_fp(stderr);
    OpenSSL_add_all_algorithms();
    //ssl context
    ctx = SSL_CTX_new(method);
    SSL_CTX_set_ecdh_auto(ctx,1);
    //ssl set private key and cert
    SSL_CTX_use_certificate_file(ctx,"bhubler_cs447.pem",SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx,"bhubler_cs447.pem",SSL_FILETYPE_PEM);
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
    SSL *sck;
    sck = SSL_new(ctx);
    SSL_set_fd(sck, clntsck);
    int sslaccept = SSL_accept(sck);
    if(sslaccept < 0)
    {
        // cout<<"Error: "<<to_string(sslaccept)<<":"<<strerror(errno)<<endl;
        // ERR_print_errors_fp(stderr);
        // close(clntsck);
        // throw runtime_error("SSL Socket Accept error\n");
    }
    ShowCerts(sck);



    bool listening = true;
    RTSPHeaders headers = RTSPHeaders();
    //Create Unauthorized header
    headers.Headers[(int)HEADER::UNAUTHORIZED].Authenticate = "Basic realm=\"CS447F18\"";
    vector<std::thread> player;
    bool uservalid = false;
    bool authfail = false;
    bool rcvconnected = false;
    int authretry = 0;
    string recvIPAddress = "";
    int recvPort = 0;
    string hostname = GetHostName();    

    string clientIP = inet_ntoa(socketaddress.sin_addr);
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getsockname(clntsck, (struct sockaddr *)&addr, &addr_size);
    string serverIP = inet_ntoa(addr.sin_addr);
    string serverPort = to_string(ntohs(addr.sin_port));
    sControl.AddClient(clntsck);
    char buffer[BUFFERSIZE];
    memset(buffer, 0, BUFFERSIZE);
    headers.CSeq = 0;
    headers.Headers[(int)HEADER::CONNECTION].Date = GetCurrentTimeStamp();
    cout<<"Opening RTSP Control Thread for client IP: "<<clientIP<<endl;
    cout<<" Socket: "<<sck<<endl;
    RTSPSendResponse(sck,200,headers,HEADER::CONNECTION,serverIP,clientIP,"CONNECT");
    int lostconn = 0;
    vector<string> msg;
    vector<string> setupmsg;
    while(listening && lostconn < 10 && authretry < 2)
    {
        int rcvdmsglength;
        try
        {
            // rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
            rcvdmsglength = SSL_read(sck,buffer,BUFFERSIZE);
        }
        catch(exception er)
        {
            cout<<er.what();
        }
        string rcvdmsg(buffer);
        while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
        {
            memset(buffer, 0, BUFFERSIZE);
            // rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
            rcvdmsglength = SSL_read(sck,buffer,BUFFERSIZE);
            rcvdmsg += buffer;
        }
        memset(buffer, 0, BUFFERSIZE);

        bool cseqvalid = false;
        if(regex_match(rcvdmsg,regex("teardown rtsp:\\/\\/([0-9a-z]){1}([\\-0-9a-z]){0,}(\\/){0,1} rtsp\\/2.0(\\s){0,}",regex::icase)) || 
           regex_match(rcvdmsg,regex("teardown rtsp:\\/\\/([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})(\\/){0,1} rtsp\\/2.0(\\s){0,}")))
        {
            lostconn = 0;
            bool dataentry = true;
            int teardowncseq;
            cseqvalid = false;
            do
            {
                int rcvdmsglength;
                memset(buffer, 0, BUFFERSIZE);
                // rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                rcvdmsglength = SSL_read(sck,buffer,BUFFERSIZE);
                string rcvdmsg(buffer);
                while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
                {
                    memset(buffer, 0, BUFFERSIZE);
                    // rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                    rcvdmsglength = SSL_read(sck,buffer,BUFFERSIZE);
                    rcvdmsg += buffer;
                }
                rcvdmsglength = rcvdmsg.length();
                if(regex_match(rcvdmsg,regex("^(cseq:){1}( ){0,}[0-9]{1,5}(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(cseq:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");                    
                    if(stoi(trim(rcvdmsg)) == (headers.CSeq + 1))
                    {
                        cseqvalid = true;
                        teardowncseq = stoi(trim(rcvdmsg));
                    }
                }
                // if((rcvdmsglength <= 2) && ((rcvdmsg[0] == 13 && rcvdmsg[1] == 10) || (rcvdmsg[0] == 13) || (rcvdmsg[0] == 10)))
                if(regex_match(rcvdmsg,regex("^(\\s){0,2}",regex::icase)))
                {
                    dataentry = false;
                }
                cout<<"Message:"<<rcvdmsg<<"|Length:"<<rcvdmsglength<<endl;
            }while(dataentry);
            if(uservalid)
            {

                if(cseqvalid)
                {
                    headers.CSeq = teardowncseq;
                    RTSPSendResponse(sck,200,headers,HEADER::TEARDOWN,serverIP,clientIP,"TEARDOWN");
                    int udpsck = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
                    struct sockaddr_in saddress;
                    saddress.sin_family = AF_INET; 
                    saddress.sin_addr.s_addr = inet_addr(recvIPAddress.c_str());
                    saddress.sin_port = htons(recvPort); 
                    string sending = "teardown";
                    sControl.SetPlaying(clntsck,false,false,false);
                    sendto(udpsck, sending.c_str(), sending.length(), 0,(struct sockaddr *) &saddress, sizeof(saddress));
                    listening = false;
                }
                else
                {
                    RTSPSendResponse(sck,400,headers,HEADER::CONNECTION,serverIP,clientIP,"TEARDOWN");  
                }
            }
            else
            {
                RTSPSendResponse(sck,401,headers,HEADER::UNAUTHORIZED,serverIP,clientIP,"TEARDOWN");
            }
            lostconn = 0;
        }
        else if(regex_match(rcvdmsg,regex("setup rtsp:\\/\\/([0-9a-z]){1}([\\-0-9a-z]){0,}(\\/){0,1} rtsp\\/2.0(\\s){0,}",regex::icase)) ||
                regex_match(rcvdmsg,regex("setup rtsp:\\/\\/([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})(\\/){0,1} rtsp\\/2.0(\\s){0,}")))
        {
            cseqvalid = false;
            bool transportvalid = false;
            lostconn = 0;
            bool dataentry = true;
            int cseq = 0;
            do
            {
                int rcvdmsglength;
                memset(buffer, 0, BUFFERSIZE);
                // rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                rcvdmsglength = SSL_read(sck,buffer,BUFFERSIZE);
                string rcvdmsg(buffer);
                while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
                {
                    memset(buffer, 0, BUFFERSIZE);
                    // rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                    rcvdmsglength = SSL_read(sck,buffer,BUFFERSIZE);
                    rcvdmsg += buffer;
                }
                rcvdmsglength = rcvdmsg.length();
                if(regex_match(rcvdmsg,regex("^(cseq:){1}( ){0,}[0-9]{1,5}(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(cseq:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");                    
                    cseq = stoi(trim(rcvdmsg));
                    headers.CSeq = cseq;
                    cseqvalid = true;                    
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
                    transportvalid = true;
                }
                if(regex_match(rcvdmsg,regex("(authorization:){1}( ){0,}(basic)( ){1,}.{1,}(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(authorization:){1}( ){0,}(basic)( ){1,}",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");
                    string auth;
                    auth = DecodeBase64(rcvdmsg);
                    vector<string> unpwd;
                    StringSplit(auth,unpwd,':');
                    if(unpwd.size() == 2)
                    {
                        string username = unpwd[0];
                        string password = unpwd[1];
                        uservalid = ValidateUser(username,password);
                        if(!uservalid)
                        {
                            authfail = true;
                        }
                    }
                }
                // if((rcvdmsglength == 2 && rcvdmsg[0] == 13 && rcvdmsg[1] == 10))
                if(regex_match(rcvdmsg,regex("^(\\s){0,2}",regex::icase)))
                {
                    dataentry = false;
                }
            }while(dataentry);
            if(uservalid)
            {
                authretry = 0;
                if(transportvalid && cseqvalid)
                {
                    RTSPSendResponse(sck,200,headers,HEADER::SETUP,serverIP,clientIP,"SETUP");
                    recvIPAddress = headers.Headers[(int)HEADER::SETUP].TransportInfo.DestAddress;
                    recvPort = stoi(headers.Headers[(int)HEADER::SETUP].TransportInfo.DestPort);
                    //Add new thread if first time
                    if(!rcvconnected)
                    {
                        player.push_back(thread(RTSPPlay,clntsck,recvIPAddress,recvPort));
                        rcvconnected = true;
                    }
                }
                else if(!cseqvalid)
                {
                    RTSPSendResponse(sck,456,headers,HEADER::CONNECTION,serverIP,clientIP,"SETUP");
                }
                else if(!transportvalid)
                {
                    RTSPSendResponse(sck,461,headers,HEADER::CONNECTION,serverIP,clientIP,"SETUP");
                }
            }
            else
            {
                if(authfail)
                {
                    RTSPSendResponse(sck,403,headers,HEADER::FORBIDDEN,serverIP,clientIP,"SETUP");
                    authretry++;
                }
                else
                {
                    RTSPSendResponse(sck,401,headers,HEADER::UNAUTHORIZED,serverIP,clientIP,"SETUP");
                }
            }
        }
        else if(regex_match(rcvdmsg,regex("play rtsp:\\/\\/([0-9a-z]){1}([\\-0-9a-z]){0,}(\\/){0,1} rtsp\\/2.0(\\s){0,}",regex::icase)) ||
                regex_match(rcvdmsg,regex("play rtsp:\\/\\/([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})(\\/){0,1} rtsp\\/2.0(\\s){0,}")))
        {
            cseqvalid = false;
            int playseq;
            bool setsensor = false;
            lostconn = 0;
            bool dataentry = true;
            do
            {
                int rcvdmsglength;
                memset(buffer, 0, BUFFERSIZE);
                // rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                rcvdmsglength = SSL_read(sck,buffer,BUFFERSIZE);
                string rcvdmsg(buffer);
                while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
                {
                    memset(buffer, 0, BUFFERSIZE);
                    // rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                    rcvdmsglength = SSL_read(sck,buffer,BUFFERSIZE);
                    rcvdmsg += buffer;
                }
                rcvdmsglength = rcvdmsg.length();
                if(regex_match(rcvdmsg,regex("^(cseq:){1}( ){0,}[0-9]{1,5}(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(cseq:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");                    
                    if(stoi(trim(rcvdmsg)) == (headers.CSeq + 1))
                    {
                        cseqvalid = true;
                        playseq = stoi(trim(rcvdmsg));
                    }
                }
                if(regex_match(rcvdmsg,regex("(sensor:){1}( ){0,}((([otp*]){1},([otp]){1},([otp]){1})|(([otp*]){1},([otp]){1})|(([otp*]){1}))(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(sensor:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");
                    headers.Headers[(int)HEADER::PLAY].SetSensor(trim(rcvdmsg));
                    setsensor = true;
                }
                // if(rcvdmsglength == 2 && rcvdmsg[0] == 13 && rcvdmsg[1] == 10)
                if(regex_match(rcvdmsg,regex("^(\\s){0,2}",regex::icase)))
                {
                    dataentry = false;
                }
            }while(dataentry);
            if(uservalid)
            {
                if(cseqvalid)
                {
                    if(!setsensor)
                    {
                        headers.Headers[(int)HEADER::PLAY].SetSensor("*");
                    }
                    headers.CSeq = playseq;
                    cout<<"Socket: "<<to_string(clntsck)<<endl;
                    sControl.SetPlaying(clntsck,headers.Headers[(int)HEADER::PLAY].Sensors[(int)SENSOR::OXYGEN],headers.Headers[(int)HEADER::PLAY].Sensors[(int)SENSOR::TEMPERATURE],headers.Headers[(int)HEADER::PLAY].Sensors[(int)SENSOR::PRESSURE]);
                    RTSPSendResponse(sck,200,headers,HEADER::PLAY,serverIP,clientIP,"PLAY");
                }
                else
                {
                    RTSPSendResponse(sck,456,headers,HEADER::CONNECTION,serverIP,clientIP,"PLAY");
                }         
            }
            else
            {
                RTSPSendResponse(sck,401,headers,HEADER::UNAUTHORIZED,serverIP,clientIP,"PLAY");
            }
            lostconn = 0;
        }
        else if(regex_match(rcvdmsg,regex("pause rtsp:\\/\\/([0-9a-z]){1}([\\-0-9a-z]){0,}(\\/){0,1} rtsp\\/2.0(\\s){0,}",regex::icase)) ||
                regex_match(rcvdmsg,regex("pause rtsp:\\/\\/([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})(\\/){0,1} rtsp\\/2.0(\\s){0,}")))
        {
            cseqvalid = false;
            int pauseseq;
            lostconn = 0;
            bool dataentry = true;
            do
            {
                int rcvdmsglength;
                memset(buffer, 0, BUFFERSIZE);
                // rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                rcvdmsglength = SSL_read(sck,buffer,BUFFERSIZE);
                string rcvdmsg(buffer);
                while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
                {
                    memset(buffer, 0, BUFFERSIZE);
                    // rcvdmsglength = recv(sck,buffer,BUFFERSIZE,0);
                    rcvdmsglength = SSL_read(sck,buffer,BUFFERSIZE);
                    rcvdmsg += buffer;
                }
                rcvdmsglength = rcvdmsg.length();
                if(regex_match(rcvdmsg,regex("^(cseq:){1}( ){0,}[0-9]{1,5}(\\s){0,}",regex::icase)))
                {
                    rcvdmsg = regex_replace(rcvdmsg,regex("^(cseq:)",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\n",regex::icase),"");
                    rcvdmsg = regex_replace(rcvdmsg,regex("\\r",regex::icase),"");  
                    if(stoi(trim(rcvdmsg)) == (headers.CSeq + 1))
                    {
                        cseqvalid = true;
                        pauseseq = stoi(trim(rcvdmsg));
                    }
                }
                // if(rcvdmsglength == 2 && rcvdmsg[0] == 13 && rcvdmsg[1] == 10)
                if(regex_match(rcvdmsg,regex("^(\\s){0,2}",regex::icase)))
                {
                    dataentry = false;
                }
            }while(dataentry);
            if(uservalid)
            {
                if(cseqvalid)
                {
                    headers.CSeq = pauseseq;
                    RTSPSendResponse(sck,200,headers,HEADER::PAUSE,serverIP,clientIP,"PAUSE");
                    sControl.SetPlaying(clntsck,false,false,false);
                }   
                else
                {
                    RTSPSendResponse(sck,456,headers,HEADER::CONNECTION,serverIP,clientIP,"PAUSE");
                } 
            }
            else
            {
                RTSPSendResponse(sck,401,headers,HEADER::UNAUTHORIZED,serverIP,clientIP,"PAUSE");
            }    
        }
        else if(rcvdmsglength == 0)
        {
            lostconn++;
        }
        else
        {
            RTSPSendResponse(sck,400,headers,HEADER::CONNECTION,serverIP,clientIP,"UNKNOWN");
        }
    }
    if(!uservalid)
    {
        RTSPSendResponse(sck,410,headers,HEADER::UNAUTHORIZED,serverIP,clientIP,"SETUP");
    }
    cout<<"Closing RTSP Control Thread for client IP: "<<clientIP<<endl;
    sControl.DisconnectClient(clntsck);
    for(uint i = 0; i < player.size(); i++)
    {
        player[i].join();
    }
    close(clntsck);
}
// int cs447::RTSPSendResponse(int &_ClientSocket, int _ResponseCode, RTSPHeaders &_Headers, HEADER _Header, string _ServerIP, string _ClientIP, string _Command)
int cs447::RTSPSendResponse(SSL *_ClientSocket, int _ResponseCode, RTSPHeaders &_Headers, HEADER _Header, string _ServerIP, string _ClientIP, string _Command)
{
    string msg = "RTSP/2.0 ";
    string Description = "";
    _Headers.Headers[(int)_Header].Date = GetCurrentTimeStamp();
    switch (_ResponseCode)
    {
        case 200:
            Description = "200 OK";
            msg += Description + "\r\n";
            msg += _Headers.PrintHeader(_Header);
            break;
        case 400:
            Description = "400 Bad Request";
            msg += Description + "\r\n";
            msg += _Headers.PrintHeader(_Header);
            break;
        case 401:
            Description = "401 Unauthorized";
            msg += Description + "\r\n";
            msg += _Headers.PrintHeader(_Header);
            break;
        case 403:
            Description = "403 Forbidden";
            msg += Description + "\r\n";
            msg += _Headers.PrintHeader(_Header);
            break;
        case 410:
            Description = "410 Gone";
            msg += Description + "\r\n";
            break;
        case 456:
            Description = "456 Header Field Not Valid for Resource";
            msg += Description + "\r\n";
            msg += _Headers.PrintHeader(_Header);
            break;
        case 461:
            Description = "461 Unsupported Transport";
            msg += Description + "\r\n";
            msg += _Headers.PrintHeader(_Header);
            break;
        default:
            Description = to_string(_ResponseCode);
            msg += Description + "\r\n";
            break;
    }    
    thread(ServerLog,_ServerIP,_ClientIP,_Command,Description).detach();
    int result = SSL_write(_ClientSocket,msg.c_str(),msg.size());
    return result;
    // return send(_ClientSocket,msg.c_str(),msg.size(),0);
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
        _Client = sControl.GetClient(_Socket);
        if(_Client->pplaying || _Client->tplaying || _Client->oplaying)
        {
            sendto(udpsck, msg.c_str(), msg.length(), 0,(struct sockaddr *) &saddress, sizeof(saddress));
        }
        this_thread::sleep_for(chrono::seconds(3));
    }
}
void cs447::SensorIncrement(bool &_Playing, int &_Counter, int &_Size)
{
    std::chrono::seconds duration(3);
    while(true)
    {
        if(_Playing)
        {
            _Counter++;
        }
        if(_Counter >= _Size)
        {
            _Counter = 0;
        }
        this_thread::sleep_for(duration);
    }
}