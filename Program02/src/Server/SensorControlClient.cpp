#include "SensorControlClient.hpp"

using namespace std;
using namespace cs447;

SensorControlClient::SensorControlClient(int _Socket)
{
    socket = _Socket;
    oplaying = false;
    tplaying = false;
    pplaying = false;
}