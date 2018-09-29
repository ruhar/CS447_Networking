#include <iostream>
#include <sys/socket.h>
#include "client.hpp"

enum {SEND=1,CHECK,EXIT};
using namespace std;
using namespace P01;
int main(int argc, char const *argv[])
{    
    string input = "";
    int inputValue = 0;
    while(inputValue != 3)
    {
        cout<<"Welcome to Haddocks Mail Client\n"<<endl;
        cout<<"1. Send Email"<<endl;
        cout<<"2. Check Email"<<endl;
        cout<<"3. Exit"<<endl;
        getline(cin,input);
        cout<<endl;
        if(input == "1" || input == "2" || input == "3")
        {
            inputValue = stoi(input);
        }
        else
        {
            inputValue = 0;
        }
        switch (inputValue)
        {
            case SEND:
                //Run TCP Send EMail Client
                SMTPSendEmail("127.0.0.1","8100");
                break;
            case CHECK:
                //Run UDP Check EMail Client
                break;
            case EXIT:
                cout<<"Thank you for using Haddocks Mail Client by Dr. Calculus."<<endl;
                break;
            default:
                cout<<"Invalid input, enter only 1, 2, or 3 followed by enter.\n\n"<<endl;
                break;
        }
    }
    return 0;
}