#ifndef rstpheaders_hpp
#define rstpheaders_hpp
#include <string>
#include <vector>

namespace cs447
{
    enum class SENSOR{OXYGEN=0, PRESSURE, TEMPERATURE};
    class rstpheaders
    {
        public:
            std::string Sensor;
            std::string CSeq;
            std::string Date;
            std::string Transport;
            std::string PrintHeaders();
            std::vector<bool> Sensors;
            bool SetSensor(std::string _Sensors);
            rstpheaders();
            ~rstpheaders();
    };
}

#endif