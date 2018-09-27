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
#include <regex>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>

using namespace std;
using namespace P01;

enum EMAIL {DATE = 0,FROM,TO,SUBJECT,MESSAGE};
// const DATE = 0;
// const FROM = 1;
// const TO = 2;


void P01::Hello()
{
    cout<<"Welcome to the electronic age Captain!\n";
}

void P01::Goodbye()
{
    cout<<"Thank you for using Dr. Calculus's mail services!\n";
    
}

void P01::SMTPServer(int _Port)
{
    //Create mail db directory structure
    char buff[FILENAME_MAX];
    getcwd(buff,FILENAME_MAX);
    cout<<"Directory: "<<buff<<endl;
    string path(buff);
    path+="/db";
    mkdir(path.c_str(),0777);

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

    string msg = hostname;
    msg += " Haddock's SMTP Mail Service, ready at ";
    msg += GetCurrentTimeStamp();
    SMTPSendResponse(&sckaccept,220,msg);
    int helo_rsp = 0;
    int mailfrom_rsp = 0;
    int rcptto_rsp = 0;
    string email[5];
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
                    msg += rcvBuffer[i];
                }
            }
            cout<<"Msg Received: "<<msg<< " Length: " << msg.length()<<endl; 
            vector<string>cmd;
            string cmd_entry = "";
            int firstSpace = msg.find_first_of(' ');
            if(firstSpace < 0)
            {
                cmd.push_back(msg);
            }
            else
            {
                cmd.push_back(msg.substr(0,firstSpace));
                cmd.push_back(msg.substr(firstSpace + 1, msg.length()));
            }
            for(int i = 0; i < (int)cmd.size(); i++)
            {
                cout<<"CMD["<< i << "]: "<<cmd[i]<<endl;
            }

            cout<<":"<<msg<<":"<<endl;
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
                SMTPSendResponse(&sckaccept,502);
            }            
            else if(StrToUpper(cmd[0]) == "MAIL")
            {
                if(helo_rsp > 0)
                {
                    if(cmd.size() > 1)
                    {
                        string mailFrom[2];
                        int firstColon = cmd[1].find_first_of(':'); 
                        if(firstColon > 0)
                        {
                            mailFrom[0] = cmd[1].substr(0,firstColon);
                            if(StrToUpper(mailFrom[0]) == "FROM")
                            {
                                if((cmd[1].length() - 1) - firstColon > 0)
                                {
                                    mailFrom[1] = cmd[1].substr(firstColon + 1,cmd[1].length());
                                    string mailaddr = trim(mailFrom[1]);
                                    regex pattern("<+[\\w._%+-]+@[447f18.edu]+>", std::regex_constants::icase);
                                    if(regex_match(mailaddr,pattern))
                                    {
                                        email[FROM] = mailaddr;
                                        SMTPSendResponse(&sckaccept,250,mailaddr);
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
                        SMTPSendResponse(&sckaccept,501,"Argument missing");
                    }
                }
                else
                {
                    SMTPSendResponse(&sckaccept,503,"Send HELO first");
                }
            }
            else if(regex_match(msg,regex("^(RCPT)((.)+)?",std::regex_constants::icase)))
            {
                cout<<"1:"<<msg<<":"<<endl;
                cout<<"Test 1"<<endl;
                if(mailfrom_rsp > 0)
                {
                    cout<<"Test 2"<<endl;
                    cout<<"2:"<<msg<<":"<<endl;
                    if(regex_match(msg,regex("^(RCPT(\\s){1,}TO:)+(<)?([\\w._%+-])+@+(447f18.edu)+(>)?",std::regex_constants::icase)))
                    {
                        cout<<"Test 3"<<endl;
                        int firstColon = msg.find_first_of(':');
                        string mailaddr = msg.substr(firstColon + 1,msg.length() - 1);
                        cout<<"RCPT Addr: "<<mailaddr<<endl;
                        email[TO] = mailaddr;
                        SMTPSendResponse(&sckaccept,250,mailaddr);
                        rcptto_rsp = 1; 
                    }
                    else if(regex_match(msg,regex("^(RCPT(\\s){1,}TO:)",std::regex::icase)))
                    {
                        SMTPSendResponse(&sckaccept,501,"Unrecognized command");
                    }
                    else if(StrToUpper(trim(msg)) == "RCPT")
                    {
                        SMTPSendResponse(&sckaccept,501,"Argument missing");
                    }        
                    else if(regex_match(msg,regex("^(RCPT(\\s){1,})",std::regex::icase)))
                    {
                        int firstSpace = msg.find_first_of(' ');
                        SMTPSendResponse(&sckaccept,501,"Unrecognized parameter " + msg.substr(firstSpace + 1, msg.length() -1));
                    }            
                    else
                    {
                        SMTPSendResponse(&sckaccept,500);
                    }
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
            // else if(cmd[0] == "DATA")
            else if(regex_match(msg,regex("^(DATA)(\\s){0,}",std::regex::icase)))
            {
                if(rcptto_rsp > 0)
                {
                    //Proces data and save email
                    string data="";
                    bool getData = true;
                    bool setSubject = true;
                    SMTPSendResponse(&sckaccept,354,"Start mail input, end with <CRLF>.<CRLF>");
                    string strBuffer = "";
                    while(getData)
                    {
                        
                        char dataBuffer[32];
                        memset(&dataBuffer,0,sizeof(dataBuffer));
                        int receive = recv(sckaccept,dataBuffer,32,0);
                        for(int i = 0; i < receive; i++)
                        {
                            if(dataBuffer[i] == 0)
                            {
                                i = sizeof(dataBuffer);
                            }
                            else
                            {
                                strBuffer += dataBuffer[i];
                            }

                        }
                        if(strBuffer[strBuffer.length() - 1] == 13 ||
                           strBuffer[strBuffer.length() - 1] == 10)
                        //Check for subject
                        {
                            string subj="";
                            if(strBuffer.length() > 7)
                            {
                                subj = StrToUpper(strBuffer.substr(0,8));
                            }
                            if(setSubject && subj == "SUBJECT:")
                            {
                                string subject = strBuffer.substr(9,strBuffer.length());
                                regex crlf("\n");
                                subject = regex_replace(subject,crlf,"");
                                email[SUBJECT] = subject;
                            }
                            else
                            {
                                data += strBuffer;
                                setSubject = false;
                            }
                            //Check if end condition is met.
                            if(data.length() >= 5)
                            {
                                if((int)data[data.length()-5] == 13 &&
                                (int)data[data.length()-4] == 10 &&
                                (int)data[data.length()-3] == 46 &&
                                (int)data[data.length()-2] == 13 &&
                                (int)data[data.length()-1] == 10)
                                {
                                    getData = false;
                                }
                            }
                            strBuffer = "";
                        }
                    }
                    email[EMAIL::MESSAGE] = data;
                    email[DATE] = GetCurrentTimeStamp();
                    string recipient = GetUserName(email[TO]);
                    cout<<"UserName: "<<recipient<<endl;
                    string userpath = path + "/" + recipient;
                    mkdir(userpath.c_str(),0777);
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
            else
            {
                cout<<"Unknown Error"<<endl;
            }
            // cout<<"End of if block"<<endl;
        }
        close(scklisten);
        close(sckbind);
        close(sck); 
    }
    cout<<"Date: "<<email[DATE]<<endl;
    cout<<"From: "<<email[FROM]<<endl;
    cout<<"To: "<<email[TO]<<endl;
    cout<<"Subject: "<<email[SUBJECT]<<endl;
    cout<<"Message: "<<email[MESSAGE]<<endl;
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
    string time = asctime(currtime);
    regex crlf("\n");

    return regex_replace(time,crlf,"");
}
int P01::SMTPSendResponse(int *_ClientSocket, int _ResponseCode, string _Message)
{
    string msg = "";
    switch (_ResponseCode)
    {
        case 220:
            msg = "220 " + _Message + "\n";
            break;
        case 250:
            msg = "250 " + _Message + "\n";
            break;
        case 354:
            msg = "354 " + _Message + "\n";
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
    for(int i = 0; i < (int)_InputString.size(); i++)
    {
        upper += toupper(_InputString[i]);
    }
    return upper;
}
string P01::ltrim(string _InputString, char _Character)
{       
    if(_InputString.length() > 0)
    {
        while(_InputString[0] == _Character)
        {
            if(_InputString.length() > 2)
            {
                _InputString = _InputString.substr(1,_InputString.length());
            }
            else
                _InputString = "";
        }
        return _InputString;
    }
    else
        return _InputString;

}
string P01::rtrim(string _InputString, char _Character)
{
    if(_InputString.length() > 0)
    {
        while(_InputString[_InputString.length() - 1] == _Character)
        {
            if(_InputString.length() > 2)
                _InputString = _InputString.substr(0,_InputString.length() - 2);
            else
                _InputString = "";
        }
        return _InputString;
    }
    else
    {
        return _InputString;
    }
}
string P01::trim(string _InputString, char _Character)
{
    return rtrim(ltrim(_InputString,_Character),_Character);
}
string P01::GetUserName(string _EmailAddress)
{
    string username = ltrim(_EmailAddress);
    username = ltrim(username,'<');
    int atsymbol = username.find_first_of('@');
    return username.substr(0, atsymbol);
}
int P01::DeliverEmail(string _Email[], string _Path)
{
    DIR *userdir = opendir(_Path.c_str());
    struct dirent *files;
    vector<string> filenames;
    if(userdir != NULL)
    {
        while(files == readdir(userdir) != NULL)
        {
            string f = files->d_name;
            int periodFirst = f.find_first_of(.);
            f=f.substr(0,periodFirst);
            f=ltrim(f,'0');
            cout<<files->d_name<<"|"<<f<<endl;
            filenames.push_back(f);
        }
        // filenames
    }
}