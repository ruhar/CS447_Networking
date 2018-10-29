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
    cout<<"1"<<endl;
    if(regex_match(_Sensors,regex("^(([otp*]){1},([otp]){1},([otp]){1})|^(([otp*]){1},([otp]){1})|^(([otp*]){1})(\\s){0,}",regex::icase)))
    {
        cout<<"2"<<endl;
        Sensors[(int)SENSOR::OXYGEN] = false;
        Sensors[(int)SENSOR::PRESSURE] = false;
        Sensors[(int)SENSOR::TEMPERATURE] = false;
        Sensor = _Sensors;        
        set = true;
        if(regex_match(_Sensors,regex("[o*]{0,}",regex::icase)))
        {
            Sensors[(int)SENSOR::OXYGEN] = true;
            cout<<"Oxygen"<<endl;
        }
        if(regex_match(_Sensors,regex("[p*]{0,}",regex::icase)))
        {
            Sensors[(int)SENSOR::PRESSURE] = true;
            cout<<"Pressure"<<endl;
        }
        if(regex_match(_Sensors,regex("[t*]{0,}",regex::icase)))
        {
            Sensors[(int)SENSOR::TEMPERATURE] = true;
            cout<<"Temperature"<<endl;
        }
        cout<<"3"<<endl;
    }
    return set;
}