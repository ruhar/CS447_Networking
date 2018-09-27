#include <string.h>
namespace P01
{
    void Hello();
    void Goodbye();
    void SMTPServer(int _Port);
    int SMTPHelo(int *_ClientSocket, std::string _HostName, std::string _ClientInformation = "");
    void SMTPMailFrom();
    std::string GetCurrentTimeStamp();
    int SMTPSendResponse(int *_ClientSocket, int _ResponsCode, std::string _Message="");
    std::string StrToUpper(std::string _InputString);
    std::string ltrim(std::string _InputString, char _Character = ' ');
    std::string rtrim(std::string _InputString, char _Character = ' ');
    std::string trim(std::string _InputString, char _Character = ' ');
    std::string GetUserName(std::string _EmailAddress);
    int DeliverEmail(std::string _Email[],std::string _Path);
}
