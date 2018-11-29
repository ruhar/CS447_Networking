#include <string>
#include "common.hpp"
#include <regex>
#include <iostream>
#include <fstream>
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
    const int ENCODEBITS = 6;
    const int INPUTBITS = 8;
    const string ENCODEZEROBITS = "000000";
    const string INPUTZEROBITS = "00000000";
    string encoded = "";
    string b64tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    //Convert input string to bitset<8>
    vector<bitset<INPUTBITS>> input;
    vector<bitset<ENCODEBITS>> venc;
    int count = 0;
    int bitcount = 0;
    bitset<INPUTBITS> item(INPUTZEROBITS);

    for(uint j = 0; j < _ToEncode.length(); j++)
    {
        unsigned char uc = static_cast<unsigned char>(_ToEncode[j]);
        for(int i = 0;i < 8;i++)
        {
            int bit = uc % 2;
            uc = (uc - bit)/2;
            int setbit = (((count * INPUTBITS) + i) % INPUTBITS);
            item.set(setbit,bit);
            bitcount++;
            if(bitcount == 8)
            {
                input.push_back(item);
                item = bitset<INPUTBITS>(INPUTZEROBITS);
                bitcount = 0;
            }
        }       
        count++; 
    }
    if(bitcount > 0)
    {
        input.push_back(item);
    }

    //Convert bitset<8> vector to bitset<6> vector
    bitcount = 0;
    bitset<6> eitem(ENCODEZEROBITS);
    count = 0;
    for(uint i = 0; i < input.size(); i++)
    {
        string inp = input[i].to_string();
        for(int j = 0;j < 8;j++)
        {            
            int setbit = 5 - (((count * INPUTBITS) + j) % ENCODEBITS);
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
                eitem = bitset<6>(ENCODEZEROBITS);
                bitcount = 0;
            }
        }      
        count++;
    }
    if(bitcount > 0)
    {
        venc.push_back(eitem);
    }

    // Encode bitset<6> vector
    for(uint i = 0; i < venc.size();i++)
    {
        encoded += b64tbl[venc[i].to_ulong()];
    }
    //Append = as needed
    if((3-_ToEncode.length()%3) == 1)
    {
        encoded += "=";
    }
    else if((3-_ToEncode.length()%3) == 2)
    {
        encoded += "==";
    }

    return encoded;
}
string cs447::DecodeBase64(string _ToDecode)
{
    string decoded = "";
    const int INPUTBITS = 6;
    const int DECODEBITS = 8;
    const string INPUTZEROBITS = "000000";
    const string DECODEZEROBITS = "00000000";
    string b64tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    vector<bitset<INPUTBITS>> input;

    int bitcount = 0;
    for(uint i = 0; i < _ToDecode.length(); i++)
    {
        uint index = b64tbl.find_first_of(_ToDecode[i]);
        if(index < 65)
        {
            bitset<INPUTBITS> item(INPUTZEROBITS);
            
            for(int i = 0;i < INPUTBITS;i++)
            {
                int bit = index % 2;
                index = (index - bit)/2;
                int setbit = i;
                item.set(setbit,bit);
                bitcount++;
                if(bitcount == INPUTBITS)
                {
                    input.push_back(item);
                    item = bitset<INPUTBITS>(INPUTZEROBITS);
                    bitcount = 0;
                }
            }       
            if(bitcount > 0)
            {
                input.push_back(item);
            }
        }
    }
    //Convert bitset<6> to ASCII String
    bitset<DECODEBITS> ditem(DECODEZEROBITS);
    bitcount = 0;
    for(uint i = 0; i < input.size(); i++)
    {
        string inp = input[i].to_string();
        for(uint j = 0;j < inp.length(); j++)
        {
            int setbit = 7 - ((i * INPUTBITS) + j) % DECODEBITS;
            int bit = 0;
            if(inp[j] == '1')
            {
                bit = 1;
            }
            ditem.set(setbit,bit);
            bitcount++;
            if(bitcount == DECODEBITS)
            {
                decoded += (char)ditem.to_ulong();
                ditem = bitset<DECODEBITS>(DECODEZEROBITS);
                bitcount = 0;
            }
        }
    }
    if(bitcount > 0)
    {
        decoded += (char)ditem.to_ulong();
    }
    return decoded;
}
bool cs447::ValidateUser(string _Username, string _Password)
{
    bool validuser = false;
    string ePassword = EncodeBase64("CS447" + _Password);
    string path = "./bin/.passwords/." + _Username + "_pass";

    ifstream rfile(path,ios::in);
    if(rfile.good())
    {
        //check for valid password
        string password;
        getline(rfile,password);
        if(password == ePassword)
        {
            validuser = true;
        }
        rfile.close();
    }
    else
    {
        //Create password file
        ofstream wfile(path,ios::out);
        wfile << ePassword << endl;
        wfile.close();
        validuser = true;
    }
    return validuser;
}
