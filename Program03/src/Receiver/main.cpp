#include <iostream>
#include <string>
#include "common.hpp"
#include "receiver.hpp"
#include <stdexcept>

using namespace std;
using namespace cs447;

int main(int argc, char const *argv[])
{
    try
    {
        Hello();
        bool valid = ValidateUser("bhubler","EatMyCock");
        if(valid)
        {
            cout<<"Good to go bitches!"<<endl;
        }
        else
        {
            cout<<"Crash and burn mother fucker!!!"<<endl;
        }
        RTSPReceiverClient(stoi(argv[1]));
    }
    catch(const runtime_error &e)
    {
        cout<<e.what()<<endl;
    }
        Goodbye();    
    return 0;
}