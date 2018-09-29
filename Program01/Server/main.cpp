#include <iostream>
#include "server.hpp"
#include <sys/socket.h>
#include <thread>
#include <future>
#include <ctime>

using namespace std;
int main(int argc, char const *argv[])
{    
    P01::Hello();
    auto t = async(launch::async,P01::SMTPServer,stoi(argv[1]));
    auto u = async(launch::async,P01::UDPServer,stoi(argv[1]));
    t.get();
    u.get();    
    P01::Goodbye();
    return 0;
}