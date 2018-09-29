#include <iostream>
#include <string>
#include "client.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex>
#include <unistd.h>

using namespace std;
using namespace P01;
enum {RECEIVEBUFFERSIZE = 128};
int P01::SMTPSendEmail(string _Hostname, string _Port)
{
    int csck = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(_Hostname.c_str());
    serverAddr.sin_port = htons(stoi(_Port));
    
    int csckConnect = connect(csck,(struct sockaddr *) &serverAddr,sizeof(serverAddr));
    if(csckConnect < 0)
    {
        cout<<"Connection failed."<<endl;
        return -1;
    }
    string input;
    bool dataEntry = false;
    while(StrToUpper(input) != "QUIT")
    {
        string msg = ""; 
        if(!dataEntry)
        {
            char messageBuffer[RECEIVEBUFFERSIZE];
            memset(messageBuffer,0,RECEIVEBUFFERSIZE);
            int recvMessageSize = recv(csck,messageBuffer,RECEIVEBUFFERSIZE,0);
            // string test(messageBuffer);
            // cout<<test<<endl;
            // cout<<recvMessageSize<<endl;
            // cout<<"Buff:"<<messageBuffer<<endl;
            msg += rtrim(messageBuffer,'\t');
            while(recvMessageSize > 0 && !(recvMessageSize < RECEIVEBUFFERSIZE))
            {
                recvMessageSize = recv(csck,messageBuffer,RECEIVEBUFFERSIZE,0);
                msg += rtrim(messageBuffer,'\t');
                // cout<<msg<<endl;
                memset(messageBuffer,0,RECEIVEBUFFERSIZE);
            }
        }
        if(msg.substr(0,3) == "354")
        {
            cout<<"data entry mode"<<endl;
            dataEntry = true;
        }
        cout<<msg;
        std::getline(std::cin,input);
        // input += "\n";
        cout<<"|"<<input<<"|"<<endl;
        if(input == ".")
        {
            dataEntry = false;
            cout<<"exit data entry mode";
            input = "\n.\n";
        }
        char msgSend[input.length()];
        strcpy(msgSend,input.c_str());
        send(csck,msgSend,sizeof(msgSend),0);
    }
    return 0;
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