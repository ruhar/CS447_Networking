#include <string>
#include "smtpargs.hpp"
#include "server.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdexcept>

#include <unistd.h>
#include <vector>
#include <netdb.h>
#include <ctime>
#include <regex>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <iomanip>
#include <thread>

using namespace std;
using namespace cs447;

static int sckaccept;

// struct smtpargs
// {
//     int socket;
//     struct sockaddr_in caddress;
// };
void cs447::Hello()
{
    cout<<"Welcome to the electronic age Captain!\n";
}
void cs447::Goodbye()
{
    cout<<"Thank you for using Dr. Calculus's mail services!\n";    
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

    while(true)
    {
        pthread_t server_thread;
        struct sockaddr_in caddress;
        socklen_t caddr_length = sizeof(caddress);
        sckaccept = accept(sck,(struct sockaddr *) &caddress,&caddr_length);
        if(sckaccept < 0)
        {
            cout<<"Client connection refused."<<endl;
        }
        else
        {
            smtpargs sckinfo;
            sckinfo.address = caddress;
            sckinfo.socket = sckaccept;
            pthread_create(&server_thread,NULL,&SMTPServerHandler,(void *) &sckinfo);
        }
    }
}
void *cs447::SMTPServerHandler(void *_sckinfo)
{
    smtpargs *sckinfo = (smtpargs *)_sckinfo;
    SMTPSendResponse(sckinfo->socket,100);
    cout<<"Threading working"<<endl;
    cout<<"caddress length: "<<sizeof(sckinfo->address);
    cout<<"client address: "<<inet_ntoa(sckinfo->address.sin_addr)<<endl;
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
        char msgSend[msg.length()];
        strcpy(msgSend,msg.c_str());
        return send(_ClientSocket,msgSend,sizeof(msgSend),0);
    }
    else
    {
        return -1;
    }
}

