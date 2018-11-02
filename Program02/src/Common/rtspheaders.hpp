#ifndef rtspheaders_hpp
#define rtspheaders_hpp
#include <string>
#include <vector>

namespace cs447
{
    enum class SENSOR{OXYGEN=0, PRESSURE, TEMPERATURE};
    class Transport
    {
        public:
            std::string Protocol;
            std::string Transmission;
            std::string DestAddress;
            std::string DestPort;
            std::string SrcAddress;
            std::string SrcPort;
            std::string ToString();
            Transport();
    };
    class rtspheaders
    {
        public:
            std::string Sensor;
            std::string CSeq;
            std::string Date;
            Transport TransportInfo;
            std::string PrintHeaders();
            std::vector<bool> Sensors;
            bool SetSensor(std::string _Sensors);
            rtspheaders();
            ~rtspheaders();
    };
}

#endif