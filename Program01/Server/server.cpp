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
#include <iomanip>
#include <pthread.h>
#include <thread>

const int BUFFERSIZE = 128;

using namespace std;
using namespace P01;

enum EMAIL {DATE = 0,FROM,TO,SUBJECT,MESSAGE};
enum {GET=0,HOST,COUNT};
enum {RESPONSE=0,SERVER,LASTMODIFIED,RETRIEVECOUNT,CONTENTTYPE,MESSAGECOUNT};
// const DATE = 0;
// const FROM = 1;
// const TO = 2;
struct arg_struct
{
    int arg1;
    struct sockaddr_in arg2;
};
string path;
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
    // cout<<"Directory: "<<buff<<endl;
    path = buff;
    path+="/db";
    mkdir(path.c_str(),0777);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(_Port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    int sck = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    int sckbind = bind(sck,(struct sockaddr *)&address,sizeof(address));
    if(sckbind < 0)
    {
        cout<<_Port<<" TCP port is not available, restart with a new port."<<endl;
    }
    else
    {
        cout<<"Waiting for TCP Connections on port "<<_Port<<"\n"<<endl;
    }
    listen(sck,5);
    struct sockaddr_in clnt_addr;
    socklen_t clnt_length = sizeof(clnt_addr);
    int sckaccept;
    int *new_sck;
    while((sckaccept = accept(sck, (struct sockaddr *) &clnt_addr, &clnt_length)))
    {
        // cout<<"Socket Accept: " << sckaccept << endl;
        // cout<<"Client Information"<<endl;
        // cout<<"  Address: " << clnt_addr.sin_addr.s_addr << endl;
        // cout<<"     Port: " << clnt_addr.sin_port << endl;

        pthread_t server_thread;
        new_sck = (int*)malloc(1);
        *new_sck = sckaccept;
        struct arg_struct args;
        args.arg1 = sckaccept;
        args.arg2 = clnt_addr;

        pthread_create(&server_thread,NULL,&SMTPServerHandler,(void*) &args);
       
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
            if(_InputString.length() > 1)
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
        while((files = readdir(userdir)) != NULL)
        {
            string f = files->d_name;
            if(f != "." && f != ".." && f != "inbox")
            {
                filenames.push_back(f);
            }
        }
        sort(filenames.begin(),filenames.end());
        string lstFilename = "";
        if(filenames.size() == 0)
        {
            lstFilename = "0";
        }
        else
        {
            lstFilename = filenames[filenames.size() - 1];
        }
        ostringstream result;
        cout<<lstFilename<<endl;
        result << setfill('0') << setw(3) << to_string(stoi(lstFilename) + 1);
        string newfilename = result.str() + ".email";
        ofstream newfile;
        newfile.open(_Path + "/" + newfilename);
        newfile <<"Date: "<< _Email[DATE] << endl;
        newfile <<"From: "<< _Email[FROM] << endl;
        newfile <<"To: "<< _Email[TO] << endl;
        newfile <<"Subject: "<< _Email[SUBJECT] << endl << endl;
        newfile << _Email[MESSAGE] << endl;
        newfile.close();
        return 0;
    }
    return -1;
}
void *P01::SMTPServerHandler(void *_Arguments)
{
    struct arg_struct *args = (struct arg_struct *)_Arguments;
    struct sockaddr_in clnt_addr = args->arg2;
    int sckaccept = args->arg1;
    char ipaddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clnt_addr.sin_addr),ipaddr,INET_ADDRSTRLEN);
    cout<<"Opening SMTP Thread for client IP: "<<ipaddr<<endl;
    char hostname[255];
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
        char rcvBuffer[BUFFERSIZE];
        int receive = recv(sckaccept,rcvBuffer,BUFFERSIZE,0);
        //cout<<rcvBuffer<<endl;
        if(receive > 0)
        {
            string msg = "";
            for(int i = 0; i < receive; i++)
            {
                //Grab all visible character
                if(rcvBuffer[i]>31)
                {
                    msg += rcvBuffer[i];
                }
            }
            cout<<msg<<endl;
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
            if(StrToUpper(cmd[0]) == "QUIT")
            {
                SMTPSendResponse(&sckaccept,221);
                cout<<"Closing SMTP Thread for client IP: "<<ipaddr<<endl;
                close(sckaccept);
                sckaccept = 0;            
            }
            else if(StrToUpper(cmd[0]) == "HELO")
            {
                string ip = inet_ntoa(clnt_addr.sin_addr);
                helo_rsp = SMTPHelo(&sckaccept,hostname,"[" + ip + "]");
            }
            else if(StrToUpper(cmd[0]) == "HELP")
            {
                SMTPSendResponse(&sckaccept,502);
                email[DATE] = GetCurrentTimeStamp();
                email[SUBJECT] = "The Last Unicorn";
                email[TO] = "<b@447f18.edu>";
                email[FROM] = "<bhubler@447f18.edu>";
                email[MESSAGE] = "Dear Haddock,\n\nGlad to hear that you found the last Unicorn.  We are looking forward to your safe return.\n\nYours truly,\nTintin and Snowy.\n";
                string recipient = GetUserName(email[TO]);
                string userpath = path + "/" + recipient;
                mkdir(userpath.c_str(),0777);
                DeliverEmail(email,userpath);
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
                                    regex pattern("<?([\\w._%+-])+(@447f18.edu){1}>?", std::regex_constants::icase);
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
                if(mailfrom_rsp > 0)
                {
                    if(regex_match(msg,regex("^(RCPT(\\s){1,}TO:){1}(<)?([\\w._%+-])+@{1}(447f18.edu){1}(>)?",std::regex_constants::icase)))
                    {
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
            else if(regex_match(msg,regex("^(DATA)(\\s){0,}",std::regex::icase)))
            {
                if(rcptto_rsp > 0)
                {
                    //Proces data and save email
                    string data="";
                    bool getData = true;
                    bool setSubject = true;
                    SMTPSendResponse(&sckaccept,354);
                    string strBuffer = "";
                    while(getData)
                    {
                        
                        char dataBuffer[BUFFERSIZE];
                        memset(&dataBuffer,0,sizeof(dataBuffer));
                        int receive = recv(sckaccept,dataBuffer,BUFFERSIZE,0);
                        cout<<dataBuffer<<endl;
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
                                    data = data.substr(0,data.length() - 5);
                                }
                                
                            }
                            strBuffer = "";
                        }
                    }
                    email[EMAIL::MESSAGE] = data;
                    email[DATE] = GetCurrentTimeStamp();
                    string recipient = GetUserName(email[TO]);
                    string userpath = path + "/" + recipient;
                    mkdir(userpath.c_str(),0777);
                    if(DeliverEmail(email,userpath) >= 0)
                    {
                        SMTPSendResponse(&sckaccept,250,"Message Sent");

                    }
                    else
                    {
                        SMTPSendResponse(&sckaccept,554);    
                    }
                    mailfrom_rsp = 0;
                    rcptto_rsp = 0;
                    email[DATE] = "";
                    email[FROM] = "";
                    email[TO] = "";
                    email[SUBJECT] = "";
                    email[MESSAGE] = "";                }
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
                // cout<<"Unknown Error"<<endl;
                SMTPSendResponse(&sckaccept,500);
                // cout<<"Closing SMTP Thread for client IP: "<<ipaddr<<endl;
                // close(sckaccept);
                // sckaccept = 0;  
            }
        }
    }
    SMTPSendResponse(&sckaccept,221);
    return 0;
}
void P01::UDPServer(int _Port)
{
    //Create mail db directory structure
    char udpWorkingDirectory[FILENAME_MAX];
    getcwd(udpWorkingDirectory,FILENAME_MAX);

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(_Port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    int sck = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    bind(sck,(struct sockaddr *)&address,sizeof(address));
    cout<< "Waiting for UDP Connections on port "<<_Port<<"\n"<<endl;
    while(true)
    {
        bool bGet = false;
        bool bHost = false;
        bool bCount = false;
        char rcvBuffer[BUFFERSIZE];
        string Request[3];
        string strBuffer;
        struct sockaddr_in clnt_addr;
        socklen_t clnt_length = sizeof(clnt_addr);
        memset(rcvBuffer,0,BUFFERSIZE); 
        recvfrom(sck,rcvBuffer,BUFFERSIZE,0,(struct sockaddr *) &clnt_addr, &clnt_length);
        vector<string> input;
        strBuffer = rcvBuffer;
        StringSplit(strBuffer,input,10);
        for(int i = 0; i < (int)input.size(); i++)
        {
            if(regex_match(input[i],regex("^(get ){1}(/db/)([\\w._%+-/])+( HTTP/1.1){1}$",regex::icase)))
            {
                Request[GET] = input[i];
                bGet = true;
                cout<<"GET: " << input[i]<<endl;
            }
            else if(regex_match(input[i],regex("^(host:( ){0,}){1}(.)+",regex::icase)))
            {
                Request[HOST] = input[i];
                bHost = true;
                cout<<"HOST: " << input[i]<<endl;
            }
            else if(regex_match(input[i],regex("^(count:( ){0,}){1}([0-9])+",regex::icase)))
            {
                Request[COUNT] = input[i];
                bCount = true;
                cout<<"Count: "<< input[i]<<endl;
            }            
        }
        if(bGet && bHost && bCount)
        {
            int colonFirst = Request[COUNT].find_first_of(':');
            string sCount = Request[COUNT].substr(colonFirst + 1,Request[COUNT].length());
            sCount = trim(sCount,' ');
            int count;
            count = stoi(sCount);
            colonFirst = Request[HOST].find_first_of(':');
            string hostname = trim(Request[HOST].substr(colonFirst + 1,Request[HOST].length()));
            vector<string> parseGet;
            StringSplit(Request[GET],parseGet,32);
            string headers[6];
            headers[RESPONSE] = parseGet[2];
            headers[SERVER] = hostname;
            headers[LASTMODIFIED] = GetCurrentTimeStamp();
            headers[RETRIEVECOUNT] = to_string(count);
            headers[CONTENTTYPE] = "text/plain";
            
            string userDir = udpWorkingDirectory + rtrim(trim(parseGet[1],' '),'/');
            string userInbox = userDir + "/inbox";
            mkdir(userDir.c_str(),0777);
            mkdir(userInbox.c_str(),0777);
            vector<string> Emails;
            vector<string> EmailFilenames;
            RetrieveEmail(Emails,EmailFilenames,userDir,count);
            for(int i = 0; i < (int)Emails.size(); i++)
            {
                headers[MESSAGECOUNT] = to_string(i + 1);
                string email = headers[RESPONSE] + "\n";
                email += "Server: " + headers[HOST] + "\n";
                email += "Last-Modified: " + headers[LASTMODIFIED] + "\n";
                email += "Count: " + headers[RETRIEVECOUNT] + "\n";
                email += "Content-Type: " + headers[CONTENTTYPE] + "\n";
                email += "Message: " + headers[MESSAGECOUNT] + "\n\n";
                email += Emails[i];
                SaveRetrievedEmail(email,userInbox,EmailFilenames[i]);
                UDPSendResponse(&sck,clnt_addr,email);
            }
            char ipaddr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clnt_addr.sin_addr),ipaddr,INET_ADDRSTRLEN);
            cout<<Emails.size()<<" UDP messages sent to client IP: "<<ipaddr<<endl;
        }
    }
}
int P01::UDPSendResponse(int *_ServerSocket,struct sockaddr_in _ClientAddr,string _Message)
{
    return sendto(*_ServerSocket,_Message.c_str(),_Message.length(),0,(struct sockaddr *) &_ClientAddr,(socklen_t)sizeof(_ClientAddr));;
}

void P01::StringSplit(string _InputToSplit,vector<string> &_DelimitedOutput, char _Delimiter)
{
    string line = "";
    for(int i = 0;i < (int)_InputToSplit.length(); i++)
    {
        line += _InputToSplit[i];
        if(_InputToSplit[i] == _Delimiter)
        {
            line = regex_replace(line,regex("\n"),"");
            _DelimitedOutput.push_back(line);
            line = "";
        }
    }
    if(line != "")
    {
        line = regex_replace(line,regex("\n"),"");
        _DelimitedOutput.push_back(line);
    }
}

int P01::RetrieveEmail(vector<string> &_Emails,vector<string> &_EmailFilenames, string _Mailbox, int _Count)
{
    DIR *userdir = opendir(_Mailbox.c_str());
    struct dirent *files;
    vector<string> filenames;
    if(userdir != NULL && _Count > 0)
    {
        while((files = readdir(userdir)) != NULL)
        {
            string f = files->d_name;
            if(f != "." && f != ".." && f != "inbox")
            {
                filenames.push_back(f);
            }
        }              
        sort(filenames.begin(),filenames.end());
        for(int i = filenames.size() - 1;i >= 0; i--)
        {
            if(_Count > 0)
            {                
                ifstream f(_Mailbox + "/" + filenames[i]);
                stringstream buffer;
                buffer<<f.rdbuf();
                _EmailFilenames.push_back(filenames[i]);
                _Emails.push_back(buffer.str());
                f.close();
            }
            else
            {
                i = -1;
            }
            _Count--;
        }
        return 0;
    }
    return -1;
}
int P01::SaveRetrievedEmail(string _Email, string _Path, string _Filename)
{
    DIR *userdir = opendir(_Path.c_str());
    // struct dirent *files;
    vector<string> filenames;
    if(userdir != NULL)
    {
        ostringstream result;
        int lastPeriod = _Filename.find_last_of('.');
        result << _Path + "/" + _Filename.substr(0,lastPeriod);
        // cout<<result.str()<<endl;
        string newfilename = result.str() + ".txt";
        ofstream newfile;
        cout<<newfilename<<endl;
        newfile.open(newfilename);
        // newfile <<_Headers[RESPONSE]<<endl;
        // newfile <<"Server: "<<_Headers[HOST]<<endl;
        // newfile <<"Last-Modified: "<<_Headers[LASTMODIFIED]<<endl;
        // newfile <<"Count: "<<_Headers[COUNT]<<endl;
        // newfile <<"Content-type: "<<_Headers[CONTENTTYPE]<<endl;
        // newfile <<"Message: "<<_Headers[MESSAGE]<<endl<<endl;
        newfile <<_Email;
        newfile.close();
        return 0;
    }
    return -1;
}
