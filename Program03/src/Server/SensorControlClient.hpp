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
            bool killthread;
            SensorControlClient(int _Socket);
    };
}

#endif