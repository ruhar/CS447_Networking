#include <iostream>
#include <string>
#include "common.hpp"
#include "controller.hpp"
#include <stdexcept>
#include <regex>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


using namespace std;
using namespace cs447;

int main(int argc, char const *argv[])
{
    try
    {
        Hello();
        if(regex_match(argv[1],regex("([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})\\.([0-2]{0,1}[0-9]{0,2})")))
        {
            RTSPControlClient(argv[1], stoi(argv[2]), stoi(argv[3]));
        }
        else
        {
            hostent * record = gethostbyname(argv[1]);
            if(record == NULL)
            {
                throw runtime_error("Invalid hostname/ip address.");
            }
            in_addr * address = (in_addr *)record->h_addr;
            string ip_address = inet_ntoa(* address);   
            RTSPControlClient(ip_address, stoi(argv[2]), stoi(argv[3]));        
        }
    }
    catch(const runtime_error &e)
    {
        Goodbye();
        cout<<e.what()<<endl;
    }
    
    return 0;
}