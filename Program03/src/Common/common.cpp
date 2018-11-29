#include <string>
#include "common.hpp"
#include <regex>
#include <iostream>
#include <unistd.h>
#include <bitset>

using namespace std;

string cs447::GetCurrentTimeStamp()
{
    time_t _time = time(NULL);
    struct tm * currtime = localtime(&_time);
    string time = asctime(currtime);
    regex crlf("\n");

    return regex_replace(time,crlf,"");
}
string cs447::StrToUpper(string _InputString)
{
    string upper = "";
    for(int i = 0; i < (int)_InputString.size(); i++)
    {
        upper += toupper(_InputString[i]);
    }
    return upper;
}
string cs447::ltrim(string _InputString, char _Character)
{       
    if(_InputString.length() > 0)
    {
        while(_InputString[0] == _Character)
        {
            if(_InputString.length() > 1)
            {
                _InputString = _InputString.substr(1,_InputString.length());
            }
            else
                _InputString = "";
        }
        return _InputString;
    }
    else
        return _InputString;

}
string cs447::rtrim(string _InputString, char _Character)
{
    if(_InputString.length() > 0)
    {
        while(_InputString[_InputString.length() - 1] == _Character)
        {
            if(_InputString.length() > 2)
                _InputString = _InputString.substr(0,_InputString.length() - 2);
            else
                _InputString = "";
        }
        return _InputString;
    }
    else
    {
        return _InputString;
    }
}
string cs447::trim(string _InputString, char _Character)
{
    return rtrim(ltrim(_InputString,_Character),_Character);
}
string cs447::GetUserName(string _EmailAddress)
{
    string username = ltrim(_EmailAddress);
    username = ltrim(username,'<');
    int atsymbol = username.find_first_of('@');
    return username.substr(0, atsymbol);
}
string cs447::GetHostName()
{
    char hostname[255];
    if(gethostname(hostname,255) == 0)
    {
        string hname(hostname);
        return hname;
    }
    else
    {
        return "";
    }
}
void cs447::StringSplit(string _InputToSplit,vector<string> &_DelimitedOutput, char _Delimiter)
{
    string line = "";
    for(int i = 0;i < (int)_InputToSplit.length(); i++)
    {
        line += _InputToSplit[i];
        if(_InputToSplit[i] == _Delimiter)
        {
            line = line.substr(0,line.length() - 1);
            line = regex_replace(line,regex("\n"),"");
            _DelimitedOutput.push_back(line);
            line = "";
        }
    }
    if(line != "")
    {
        line = regex_replace(line,regex("\n"),"");
        _DelimitedOutput.push_back(line);
    }
}
string cs447::EncodeBase64(string _ToEncode)
{
    const int BITS = 6;
    const string ZEROBITS = "000000";
    string encoded = "";
    string b64tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+/";
    //Convert input string to bitset<8>
    vector<bitset<8>> input;
    vector<bitset<6>> venc;
    int count = 0;
    int bitcount = 0;
    bitset<8> item("00000000");

    for(int j = 0; j < _ToEncode.length(); j++)
    {
        unsigned char uc = static_cast<unsigned char>(_ToEncode[j]);
        cout<<"1:"<<(char)uc<<endl;
        for(int i = 0;i < 8;i++)
        {
            int bit = uc % 2;
            uc = (uc - bit)/2;
            int setbit = (((count * 8) + i) % 8);
            item.set(setbit,bit);
            bitcount++;
            if(bitcount == 8)
            {
                input.push_back(item);
                cout<<"2: "<<item.to_string()<<endl;
                item = bitset<8>("00000000");
                bitcount = 0;
            }
        }       
        count++; 
    }
    if(bitcount > 0)
    {
        input.push_back(item);
        cout<<"3: "<<item.to_string()<<endl;
    }
    cout<<"test2"<<endl;
    bitcount = 0;
    bitset<6> eitem("000000");
    count = 0;
    cout<<input.size()<<endl;
    for(int i = 0; i < input.size(); i++)
    {
        cout<<"test1"<<endl;
        string inp = input[i].to_string();
        // for(int j = 7; j >= 0; j--)
        // {
        //     cout<<"test"<<endl;
        //     int setbit = 6-(((count*8) - i) % 6);
        //     eitem.set(setbit,input[i][j]);
        //     bitcount++;
        //     if(bitcount == 6)
        //     {
        //         cout<<eitem.to_string()<<endl;
        //         venc.push_back(eitem);
        //         eitem = bitset<6>("000000");
        //         bitcount = 0;
        //     }
        // }
        for(int j = 0;j < 8;j++)
        {            
            int setbit = 5 - (((count * 8) + j) % 6);
            int bit = 0;
            if(inp[j] == '1')
            {
                bit = 1;
            }
            eitem.set(setbit,bit);
            bitcount++;
            if(bitcount == 6)
            {
                venc.push_back(eitem);
                cout<<"2: "<<eitem.to_string()<<endl;
                eitem = bitset<6>("000000");
                bitcount = 0;
            }
        }      
        count++;
    }
    for(uint i = 0; i < venc.size();i++)
    {
        cout<<"4: "<< venc[i].to_ulong()<<endl;
        encoded += b64tbl[venc[i].to_ulong()];
    }
    //Append = as needed
    if((3-_ToEncode.length()%3) == 1)
    {
        encoded += "=";
    }
    if((3-_ToEncode.length()%3) == 2)
    {
        encoded += "==";
    }
    return encoded;
}
