#include <iostream>
#include "server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <netdb.h>
#include <ctime>

using namespace std;
using namespace P01;

void P01::Hello()
{
    cout<<"What's up bitch!\n";
}

void P01::Goodbye()
{
    cout<<"Peace out hoe!\n";
}

void P01::SMTPServer(int _Port)
{
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(_Port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    char hostname[255];
    int getnameinfoValue = gethostname(hostname,sizeof(hostname));
    if(getnameinfoValue == 0)
    {
        cout<<hostname<<endl;
    }
    else
    {
        cout<<"Reverse lookup failed"<<endl;
        cout<<"Return value: "<< getnameinfoValue;
        cout<<"Error: "<<gai_strerror(getnameinfoValue)<<endl;
    }

    string test = "";
    
    int sck = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    cout<< "Socket: " << sck << endl;
    
    int sckbind = bind(sck,(struct sockaddr *)&address,sizeof(address));
    cout<< "Socket Bind: " << sckbind << endl;
    
    int scklisten = listen(sck,5);
    cout<<"Socket Listen: " << scklisten << endl;
    struct sockaddr_in clnt_addr;
    socklen_t clnt_length = sizeof(clnt_addr);
    cout<<"Client Length: "<<clnt_length<<endl;
    int sckaccept = accept(sck, (struct sockaddr *) &clnt_addr, &clnt_length);
    cout<<"Socket Accept: " << sckaccept << endl;
    cout<<"Client Information"<<endl;
    cout<<"  Address: " << clnt_addr.sin_addr.s_addr << endl;
    cout<<"     Port: " << clnt_addr.sin_port << endl;

    string msg = "220 ";
    msg += hostname;
    msg += " Haddock's SMTP Mail Service, ready at ";
    msg += GetCurrentTimeStamp() + "\n";
    int sent = send(sckaccept,msg.c_str(),msg.length() - 1,0);
    int helo_rsp = 0;
    int mailfrom_rsp = 0;
    int rcptto_rsp = 0;
    while(sckaccept > 0)
    {
        char rcvBuffer[32];
        int receive = recv(sckaccept,rcvBuffer,32,0);
        if(receive > 0)
        {
            cout<<"Receive: " << receive<<endl;
            string msg = "";
            for(int i = 0; i < receive; i++)
            {
                //Grab all visible character
                if(rcvBuffer[i]>31)
                {
                    // msg += toupper(rcvBuffer[i]);
                    msg += rcvBuffer[i];
                }
            }
            cout<<"Msg Received: "<<msg<< " Length: " << msg.length()<<endl; 
            vector<string>cmd;
            string cmd_entry = "";
            for(int i = 0; i < msg.length(); i++)
            {
                if(msg[i] != ' ')
                {
                    cmd_entry += msg[i];
                }
                else
                {
                    cmd.push_back(cmd_entry);
                    cmd_entry = "";
                }
            }
            if(cmd_entry.length() > 0)
            {
                cmd.push_back(cmd_entry);
            }            
            cout<<"CMD: "<<cmd[0]<<endl;
            if(StrToUpper(cmd[0]) == "QUIT")
            {
                close(sckaccept);
                sckaccept = 0;            
            }
            else if(StrToUpper(cmd[0]) == "HELO")
            {
                string ip = inet_ntoa(clnt_addr.sin_addr);
                helo_rsp = SMTPHelo(&sckaccept,hostname,"[" + ip + "]");
                cout<<"HELO RSP: "<<helo_rsp<<endl;
            }
            else if(StrToUpper(cmd[0]) == "HELP")
            {

            }            
            else if(StrToUpper(cmd[0]) == "MAIL")
            {
                // cout<<"HELO RSP: "<<helo_rsp<<endl;
                if(helo_rsp > 0)
                {
                    cout<<"Size: "<<cmd.size()<<endl;
                    if(cmd.size() > 1)
                    {
                        if(StrToUpper(cmd[1]) == "FROM:")
                        {
                            if(cmd.size() > 2)
                            {
                                string maildomain = "";
                                string mailrcpt = "";
                                string mailaddr = cmd[2];
                                int atposition = mailaddr.find_first_of('@');
                                maildomain = mailaddr.substr(atposition, sizeof(mailaddr) - 2);
                                mailrcpt = mailaddr.substr(1,atposition - 1);
                                cout<<"Domain: "<< maildomain << endl;
                                cout<<"Recipient: "<< mailrcpt << endl;
                                if(mailaddr[0] == '<' && mailaddr[cmd.size() - 1] == '>' && StrToUpper(maildomain) == "@447F18.EDU")
                                {
                                    mailfrom_rsp = 1;
                                }
                                else
                                {
                                    SMTPSendResponse(&sckaccept,501,"Invalid address");
                                }
                            }                            
                            else
                            {
                                SMTPSendResponse(&sckaccept,501,"Invalid address");
                            }
                        }
                        else
                        {
                            SMTPSendResponse(&sckaccept,500,"Unrecognized parameter: " + cmd[1]);
                        }
                    }
                    else
                    {
                        SMTPSendResponse(&sckaccept,501,"Argument missing");
                    }
                    
                }
                else
                {
                    SMTPSendResponse(&sckaccept,503,"Send HELO first");
                }
            }
            else if(cmd[0] == "RCPT")
            {
                if(mailfrom_rsp > 0)
                {
                    rcptto_rsp = 1;
                }
                else
                {
                    if(helo_rsp < 1)
                    {
                        SMTPSendResponse(&sckaccept,503,"Send HELO first");
                    }
                    else
                    {
                        SMTPSendResponse(&sckaccept,503,"Need MAIL FROM: first");
                    }
                }
            }
            else if(cmd[0] == "DATA")
            {
                if(rcptto_rsp > 0)
                {
                    //Proces data and save email
                }
                else
                {
                    if(helo_rsp < 1)
                    {
                        SMTPSendResponse(&sckaccept,503,"Send HELO first");
                    }
                    else if(mailfrom_rsp < 1)
                    {
                        SMTPSendResponse(&sckaccept,503,"Need MAIL FROM: first");
                    }
                    else
                    {
                        SMTPSendResponse(&sckaccept,503,"Need RCPT TO: first");
                    }
                }
            }
        }
        close(scklisten);
        close(sckbind);
        close(sck); 
    }
}

int P01::SMTPHelo(int *_ClientSocket, string _HostName, string _ClientInformation)
{    
    string msg = "Hello " + _ClientInformation;
    return SMTPSendResponse(_ClientSocket,250,msg);
}
string P01::GetCurrentTimeStamp()
{
    time_t _time = time(NULL);
    struct tm * currtime = localtime(&_time);
    return asctime(currtime);
}
int P01::SMTPSendResponse(int *_ClientSocket, int _ResponseCode, string _Message = "")
{
    string msg = "";
    switch (_ResponseCode)
    {
        case 220:
            msg = "220 ";
            break;
        case 250:
            msg = "250 " + _Message + "\n";
            break;
        case 500:
            msg = "500 Command syntax error: " +_Message + "\n";
            break;
        case 501:
            msg = "501 Argument syntax error: " + _Message + "\n";
            break;
        case 503:
            msg = "503 Bad sequence of commands: " +_Message + "\n";
            break;    
        default:
            msg += _ResponseCode + " " + _Message + "\n";
            break;
    }
    if(msg.length() > 0)
    {
        char msgSend[msg.length()];
        strcpy(msgSend,msg.c_str());
        return send(*_ClientSocket,msgSend,sizeof(msgSend),0);
    }
    else
    {
        return -1;
    }
}
string P01::StrToUpper(string _InputString)
{
    string upper = "";
    for(int i = 0; i < _InputString.size(); i++)
    {
        upper += toupper(_InputString[i]);
    }
    return upper;
}