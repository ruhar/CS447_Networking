#ifndef tcpargs_hpp
#define tcpargs_hpp
#include <netinet/in.h>

namespace cs447
{
    class tcpargs
    {
    public:
        int socket;
        sockaddr_in address; 
        tcpargs();
        ~tcpargs();
    };
}

#endif