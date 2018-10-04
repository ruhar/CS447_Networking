#include <string>
#include <vector>

using namespace std;

namespace cs447
{
    void Hello();
    void Goodbye();
    void SMTPServer(int _Port);
    int SMTPHelo(int *_ClientSocket, string _HostName, string _ClientInformation = "");
    void SMTPMailFrom();
    int SMTPSendResponse(int *_ClientSocket, int _ResponsCode, string _Message="");
    int DeliverEmail(string _Email[],string _Path);
    void *SMTPServerHandler(void *_Arguments);
    void UDPServer(int _Port);
    int UDPSendResponse(int *_ServerSocket,struct sockaddr_in _ClientAddr,string _Message);
    void StringSplit(string _InputToSplit, vector<string> &_DelimitedOutput, char _Delimiter);
    int RetrieveEmail(vector<string> &_Emails,vector<string> &_EmailFilenames,string _Mailbox, int _Count);
    int SaveRetrievedEmail(string _Email, string _Path, string _Filename);
}