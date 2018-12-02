#ifndef tcpargs_hpp
#define tcpargs_hpp
#include <netinet/in.h>
#include <openssl/ssl.h>

namespace cs447
{
    class tcpargs
    {
    public:
        SSL *ssl;
        int socket;
        sockaddr_in address; 
        tcpargs();
        ~tcpargs();
    };
}

#endif