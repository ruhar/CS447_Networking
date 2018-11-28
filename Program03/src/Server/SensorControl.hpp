#ifndef SensorControl_hpp
#define SensorControl_hpp

#include <vector>
#include "SensorControlClient.hpp"
#include <list>

namespace cs447
{
    class SensorControl
    {
    public:
        std::vector<SensorControlClient> clients;
        bool oplaying;
        bool tplaying;
        bool pplaying;
        SensorControl();
        void AddClient(int _Socket);
        void DisconnectClient(int _Socket);
        void SetPlaying(int _Socket, bool _Oxygen, bool _Temperature, bool _Pressure);
        SensorControlClient* GetClient(int _Socket);
    };
}

#endif