#ifndef SensorControlClient_hpp
#define SensorControlClient_hpp

namespace cs447
{
    class SensorControlClient
    {
        public:
            int socket;
            bool oplaying;
            bool tplaying;
            bool pplaying;
            SensorControlClient(int _Socket);
    };
}

#endif