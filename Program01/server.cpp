#include <iostream>
#include "server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <netdb.h>

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

void P01::SMTPServer()
{
    int port;
    cout<<"Port: ";
    cin>>port;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    char hostname[255];
    // string hname;
    // gethostname(hname.c_str,255);
    // cout<<hname<<endl;
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
// 220 mobile.siue.edu Microsoft ESMTP MAIL Service, 
// Version: 8.5.9600.16384 ready at  Sun, 23 Sep 2018 21:03:50 -0500
    string msg = "220 ";
    msg += hostname;
    msg += " Haddock's SMTP Mail Service, ready at ";

    int sent = send(sckaccept,msg.c_str(),msg.length() - 1,0);
    while(sckaccept > 0)
    {
        char rcvBuffer[32];
        int receive = recv(sckaccept,rcvBuffer,32,0);
        cout<<"Receive: " << receive<<endl;
        string msg = "";
        for(int i = 0; i < receive; i++)
        {
            //Grab all visible character
            if(rcvBuffer[i]>31)
                msg += toupper(rcvBuffer[i]);
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
        
        if(cmd[0] == "QUIT")
        {
            close(sckaccept);
            sckaccept = 0;            
        }
        else if(msg == "HELO")
        {
            string ip = inet_ntoa(clnt_addr.sin_addr);
            SMTPHelo(&sckaccept,hostname,"[" + ip + "]");
        }
        else if(msg == "MAIL FROM")
        {

        }
        else if(msg == "RCPT TO")
        {

        }
        else if(msg == "DATA")
        {

        }
        else if(msg == "HELP")
        {

        }
        close(scklisten);
        close(sckbind);
        close(sck);
    }
}
//#include <arpa/inet.h>

int P01::SMTPHelo(int *_ClientSocket, string _HostName, string _ClientInformation)
{
    
    string msg = "250 " + _HostName + " Hello " + _ClientInformation + "\n";
    char msgSend[msg.length()];
    strcpy(msgSend,msg.c_str());
    return send(*_ClientSocket,msgSend,sizeof(msgSend),0);
}