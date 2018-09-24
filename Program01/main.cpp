#include <iostream>
#include "server.hpp"
#include <sys/socket.h>


using namespace std;
int main(int argc, char const *argv[])
{
    /* code */
    P01::Hello();
    P01::SMTPServer();
    P01::Goodbye();
    return 0;
}