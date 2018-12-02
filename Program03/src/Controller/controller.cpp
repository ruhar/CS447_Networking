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
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>

using namespace std;
using namespace cs447;
int sequence = 0;
const int BUFFERSIZE = 64;

void ShowCerts(SSL* ssl)
{
       X509 *cert;
    char *line;
 
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        printf("Info: No client certificates configured.\n");
}

void cs447::Hello()
{
    cout<<"Hello captain, waiting for probe commands!"<<endl;
    cout<<"Usage:"<<endl;
    cout<<"The following quick command will send all headers and track proper cseq numbers."<<endl;
    cout<<"Quick Commands: setup, setupauth, play, pause, teardown."<<endl;
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
    // int sck = socket(PF_INET,SOCK_STREAM,0);
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
    SSL_CTX *ctx;
    const SSL_METHOD *method;
    // ssl init 
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    // create context
    method = TLS_client_method();
    if (!(ctx = SSL_CTX_new(method))) {
            ERR_print_errors_fp(stderr);
        exit(1);
    }
    // configure context
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_COMPRESSION);
    // create ssl instance from context
    serverinfo.ssl = SSL_new(ctx);

    // assign socket to ssl intance 
    SSL_set_fd(serverinfo.ssl, sck);
    // perform ssl handshake & connection 
    int sslconnect = SSL_connect(serverinfo.ssl);
    if(sslconnect != 1)
    {
        cout<<"Error:"<<endl;
        ERR_print_errors_fp(stderr);
        cout<<"4:"<<to_string(sslconnect)<<endl;
    }
    ShowCerts(serverinfo.ssl);
    serverinfo.socket = sck;
    serverinfo.address = saddress;
    thread sendthread (RTSPSender,serverinfo,_ReceiverPort);
    thread rcvthread (RTSPReceiver,serverinfo);

    sendthread.join();
    // rcvthread.detach();
    // rcvthread.~thread();
    close(sck);
}
void cs447::RTSPSender(tcpargs _TCPArguments, int _ReceiverPort)
{
    SSL *ssl = _TCPArguments.ssl;
    
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
            //sequence = 0;
            buffer = "setup rtsp://" + hostname + " rtsp/2.0\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "cseq:" + to_string(sequence) + "\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "transport:UDP;unicast;dest_addr=\":" + to_string(_ReceiverPort) + "\"\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "sensor:*\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else if(regex_match(buffer,regex("( ){0,}setupauth( ){0,}(\\s){0,}",regex::icase)))
        {
            //sequence = 0;
            buffer = "setup rtsp://" + hostname + " rtsp/2.0\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "cseq:" + to_string(sequence) + "\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "transport:UDP;unicast;dest_addr=\":" + to_string(_ReceiverPort) + "\"\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "sensor:*\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "Authorization: Basic aGFkZG9jazpwaXJhdGVzIQ==\r\n";
            // buffer = "Authorization: Basic aGFkZG9jazpwaXJhdGVz\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));
            buffer = "\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else if(regex_match(buffer,regex("( ){0,}play( ){0,}(\\s){0,}",regex::icase)))
        {
            buffer = "play rtsp://" + hostname + " rtsp/2.0\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "sensor:*\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "cseq:" + to_string(sequence) + "\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

        }
        else if(regex_match(buffer,regex("( ){0,}pause( ){0,}(\\s){0,}",regex::icase)))
        {
            buffer = "pause rtsp://" + hostname + " rtsp/2.0\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "cseq:" + to_string(sequence) + "\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        else if(regex_match(buffer,regex("( ){0,}teardown( ){0,}(\\s){0,}",regex::icase)))
        {
            buffer = "teardown rtsp://" + hostname + " rtsp/2.0\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "cseq:" + to_string(sequence) + "\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
            this_thread::sleep_for(chrono::milliseconds(100));

            buffer = "\r\n";
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
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
            // send(socket,buffer.c_str(),buffer.length(),0);
            SSL_write(ssl,buffer.c_str(),buffer.length());
        }
    }
}
void cs447::RTSPReceiver(tcpargs _TCPArguments)
{
    bool listening = true;
    int socket = _TCPArguments.socket;
    SSL *ssl = _TCPArguments.ssl;

    char buffer[BUFFERSIZE];
    memset(buffer, 0, BUFFERSIZE);
    while(listening)
    {
        int rcvdmsglength;
        try
        {
            // rcvdmsglength = recv(socket,buffer,BUFFERSIZE,0);
            rcvdmsglength = SSL_read(ssl,buffer,BUFFERSIZE);
        }
        catch(exception er)
        {
            cout<<er.what();
        }
        string rcvdmsg(buffer);
        while(rcvdmsglength == BUFFERSIZE && buffer[rcvdmsglength - 1] != 10)
        {
            memset(buffer, 0, BUFFERSIZE);
            // rcvdmsglength = recv(socket,buffer,BUFFERSIZE,0);
            rcvdmsglength = SSL_read(ssl,buffer,BUFFERSIZE);
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