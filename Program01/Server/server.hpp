#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>

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
    void *SMTPServerHandler(void *_Arguments);
    void UDPServer(int _Port);
    int UDPSendResponse(int *_ServerSocket,struct sockaddr_in _ClientAddr,std::string _Message);
    void StringSplit(std::string _InputToSplit, std::vector<std::string> &_DelimitedOutput, char _Delimiter);
    int RetrieveEmail(std::vector<std::string> &_Emails,std::vector<std::string> &_EmailFilenames,std::string _Mailbox, int _Count);
    int SaveRetrievedEmail(std::string _Email, std::string _Path, std::string _Filename);
}
