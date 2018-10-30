#include "rtspheaders.hpp"
#include <string>
#include <regex>
#include <vector>
#include <iostream>

using namespace std;
using namespace cs447;

rtspheaders::rtspheaders()
{
    CSeq = "";
    Date = "";
    Sensor = "";
    Transport = "";
    std::string CSeq;
    std::string Date;
    std::string Sensor;
    std::string Transport;
    std::string printheaders(); 
    Sensors.push_back(false);
    Sensors.push_back(false);
    Sensors.push_back(false);
}

rtspheaders::~rtspheaders()
{
}

string rtspheaders::PrintHeaders()
{

    string value = "";
    if(CSeq != "")
    {
        value += "CSeq: " + CSeq + "\n";
    }
    if(Date != "")
    {
        value += "Date: " + Date + "\n";
    }
    if(Transport != "")
    {
        value += "Transport: " + Transport + "\n";
    }
    if(Sensor != "")
    {
        value += "Sensor: " + Sensor + "\n";
    }
    return value;
}
bool rtspheaders::SetSensor(string _Sensors)
{
    bool set = false;
    if(regex_match(_Sensors,regex("^(([otp*]){1},([otp]){1},([otp]){1})|^(([otp*]){1},([otp]){1})|^(([otp*]){1})(\\s){0,}",regex::icase)))
    {
        Sensors[(int)SENSOR::OXYGEN] = false;
        Sensors[(int)SENSOR::PRESSURE] = false;
        Sensors[(int)SENSOR::TEMPERATURE] = false;
        Sensor = _Sensors;        
        set = true;
        if(regex_search(_Sensors,regex("[o*]{1}",regex::icase)))
        {
            Sensors[(int)SENSOR::OXYGEN] = true;
        }
        if(regex_search(_Sensors,regex("[p*]{1}",regex::icase)))
        {
            Sensors[(int)SENSOR::PRESSURE] = true;
        }
        if(regex_search(_Sensors,regex("[t*]{1}",regex::icase)))
        {
            Sensors[(int)SENSOR::TEMPERATURE] = true;
        }
    }
    return set;
}