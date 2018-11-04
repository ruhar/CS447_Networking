#include "rtspheaders.hpp"
#include <string>
#include <regex>
#include <vector>
#include <iostream>
#include "common.hpp"

using namespace std;
using namespace cs447;

Transport::Transport()
{
    Protocol = "";
    Transmission = "";
    DestAddress = "";
    DestPort = "";
    SrcAddress = "";
    SrcPort = "";
}
string Transport::ToString()
{
    string retval = "";
    if(Protocol != "")
    {
        retval += Protocol + ";";
    }
    if(Transmission != "")
    {
        retval += Transmission + ";";
    }
    if(DestAddress != ""||DestPort != "")
    {
        retval += "dest_addr=\"" + DestAddress;
        if(DestPort != "")
        {
            retval += ":" + DestPort;
        }
        retval +="\";";
    }
    if(SrcAddress != ""||SrcPort != "")
    {
        retval += "dest_addr=\"" + SrcAddress;
        if(SrcPort != "")
        {
            retval += ":" + SrcPort;
        }
        retval +="\";";
    }
    retval = rtrim(retval,';');
    return retval;
}
rtspheader::rtspheader()
{
    Date = "";
    Sensor = "";
    TransportInfo = Transport();
    std::string CSeq;
    std::string Date;
    std::string Sensor;
    // std::string Transport;
    std::string printheaders(); 
    Sensors.push_back(false);
    Sensors.push_back(false);
    Sensors.push_back(false);
}

rtspheader::~rtspheader()
{
}

bool rtspheader::SetSensor(string _Sensors)
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

RTSPHeaders::RTSPHeaders()
{
    CSeq = 0;
    Headers[(int)HEADER::SETUP] = rtspheader();
    Headers[(int)HEADER::PLAY] = rtspheader();
    Headers[(int)HEADER::PAUSE] = rtspheader();
    Headers[(int)HEADER::TEARDOWN] = rtspheader();
    Headers[(int)HEADER::CONNECTION] = rtspheader();
}
string RTSPHeaders::PrintHeader(cs447::HEADER _header)
{

    string value = "CSeq: " + to_string(CSeq) + "\r\n";

    if(Headers[(int)_header].Date != "")
    {
        value += "Date: " + Headers[(int)_header].Date + "\r\n";
    }
    if(Headers[(int)_header].TransportInfo.ToString() != "")
    {
        value += "Transport: " + Headers[(int)_header].TransportInfo.ToString() + "\r\n";
    }
    if(Headers[(int)_header].Sensor != "")
    {
        value += "Sensor: " + Headers[(int)_header].Sensor + "\r\n";
    }
    return value;
}



