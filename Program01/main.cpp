#include <iostream>
#include "server.hpp"
#include <sys/socket.h>

using namespace std;
int main(int argc, char const *argv[])
{
    P01::Hello();
    int port;
    cout<<"Port: ";
    cin>>port;
    P01::SMTPServer(port);
    P01::Goodbye();
    return 0;
}