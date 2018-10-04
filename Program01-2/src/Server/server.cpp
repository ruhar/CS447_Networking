#include <string>
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


    // int sckbind = bind()
}
