#ifndef rtspheaders_hpp
#define rtspheaders_hpp
#include <string>
#include <vector>

namespace cs447
{
    enum class SENSOR{OXYGEN=0, PRESSURE, TEMPERATURE};
    class rtspheaders
    {
        public:
            std::string Sensor;
            std::string CSeq;
            std::string Date;
            std::string Transport;
            std::string PrintHeaders();
            std::vector<bool> Sensors;
            bool SetSensor(std::string _Sensors);
            rtspheaders();
            ~rtspheaders();
    };
}

#endif