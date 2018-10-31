#include <iostream>
#include <string>
#include "common.hpp"
#include "controller.hpp"
#include <stdexcept>

using namespace std;
using namespace cs447;

int main(int argc, char const *argv[])
{
    try
    {
        Hello();
        RTSPControlClient(argv[1], stoi(argv[2]));
    }
    catch(const runtime_error &e)
    {
        Goodbye();
        cout<<e.what()<<endl;
    }
    
    return 0;
}