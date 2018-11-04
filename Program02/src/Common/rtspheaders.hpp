#ifndef rtspheaders_hpp
#define rtspheaders_hpp
#include <string>
#include <vector>

namespace cs447
{
    enum class SENSOR{OXYGEN=0, PRESSURE, TEMPERATURE};
    enum class HEADER{SETUP=0, PLAY, PAUSE, TEARDOWN, CONNECTION};
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
    class rtspheader
    {
        public:
            std::string Sensor;
            std::string Date;
            Transport TransportInfo;
            // std::string PrintHeader();
            std::vector<bool> Sensors;
            bool SetSensor(std::string _Sensors);
            rtspheader();
            ~rtspheader();
    };
    class RTSPHeaders
    {
        public:
            int CSeq;
            rtspheader Headers[5];
            std::string PrintHeader(cs447::HEADER _header);
            RTSPHeaders();
    };
}

#endif