#include <iostream>
#include <string>
#include "server.hpp"
#include "common.hpp"
#include <stdexcept>

using namespace std;
using namespace cs447;

int main(int argc, char const *argv[])
{
    Hello();    
    try
    {
        // throw runtime_error("Fuck you bitch!");
        SMTPServer(8100);
    }
    catch(const runtime_error &e)
    {
        cout<<e.what()<<endl;
        Goodbye();
    }
    
    return 0;
}
