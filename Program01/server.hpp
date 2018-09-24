#include <string.h>
namespace P01
{
    void Hello();
    void Goodbye();
    void SMTPServer();
    int SMTPHelo(int *_ClientSocket, std::string _HostName, std::string _ClientInformation = "");
    void SMTPMailFrom();
}
//Test