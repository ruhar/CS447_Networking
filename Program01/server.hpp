#include <string.h>
namespace P01
{
    void Hello();
    void Goodbye();
    void SMTPServer(int _Port);
    int SMTPHelo(int *_ClientSocket, std::string _HostName, std::string _ClientInformation = "");
    void SMTPMailFrom();
    std::string GetCurrentTimeStamp();
    int SMTPSendResponse(int *_ClientSocket, int _ResponsCode, std::string _Message);
}
