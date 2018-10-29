#include "rstpheaders.hpp"
#include <string>
#include <regex>
#include <vector>

using namespace std;
using namespace cs447;

rstpheaders::rstpheaders()
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

rstpheaders::~rstpheaders()
{
}

string rstpheaders::PrintHeaders()
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
bool rstpheaders::SetSensor(string _Sensors)
{
    bool set = false;
    if(regex_match(_Sensors,regex("^(([otp*]){1},([otp]){1},([otp]){1})|^(([otp*]){1},([otp]){1})|^(([otp*]){1})",regex::icase)))
    {
        Sensor = _Sensors;        
        set = true;
        if(_Sensors.find_first_of('*') || _Sensors.find_first_of('o') || _Sensors.find_first_of('O'))
        {
            Sensors[(int)SENSOR::OXYGEN] = true;
        }
        if(_Sensors.find_first_of('*') || _Sensors.find_first_of('p') || _Sensors.find_first_of('P'))
        {
            Sensors[(int)SENSOR::PRESSURE] = true;
        }
        if(_Sensors.find_first_of('*') || _Sensors.find_first_of('t') || _Sensors.find_first_of('T'))
        {
            Sensors[(int)SENSOR::TEMPERATURE] = true;
        }
    }
    return set;
}