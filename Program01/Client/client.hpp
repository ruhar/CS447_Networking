#include <string.h>
#include <sys/socket.h>

namespace P01
{
    int SMTPSendEmail(std::string _Hostname, std::string _Port);
    std::string StrToUpper(std::string _InputString);
    std::string ltrim(std::string _InputString, char _Character = ' ');
    std::string rtrim(std::string _InputString, char _Character = ' ');
    std::string trim(std::string _InputString, char _Character = ' ');

}
