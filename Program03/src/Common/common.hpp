    #include <string>
    #include <vector>

    using namespace std;

    namespace cs447
    {
        void StringSplit(string _InputToSplit, vector<string> &_DelimitedOutput, char _Delimiter);
        string StrToUpper(string _InputString);
        string ltrim(string _InputString, char _Character = ' ');
        string rtrim(string _InputString, char _Character = ' ');
        string trim(string _InputString, char _Character = ' ');
        string GetCurrentTimeStamp();
        string GetUserName(string _EmailAddress);
        string GetCurrentDirectory();
        string GetHostName();
        string EncodeBase64(string _ToEncode);
        string DecodeBase64(string _ToDecode);
        bool ValidateUser(string _Username, string _Password);
        void ServerLog(string _SrcIP, string _DestIP, string _ProtocolCmd, string _Code, string _Description);
    }