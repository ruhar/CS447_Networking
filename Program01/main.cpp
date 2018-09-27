#include <iostream>
#include "server.hpp"
#include <sys/socket.h>

using namespace std;
int main(int argc, char const *argv[])
{    
    P01::Hello();
    P01::SMTPServer(stoi(argv[1]));
    P01::Goodbye();
    return 0;
}