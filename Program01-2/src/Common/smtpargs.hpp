#ifndef smtpargs_hpp
#define smtpargs_hpp
#include <netinet/in.h>

namespace cs447
{
    class smtpargs
    {
    public:
        int socket;
        sockaddr_in address; 
        smtpargs();
        ~smtpargs();
    };
}

#endif