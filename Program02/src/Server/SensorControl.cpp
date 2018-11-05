#include "SensorControl.hpp"
#include "SensorControlClient.hpp"
#include <vector>

using namespace std;
using namespace cs447;

SensorControl::SensorControl()
{
    oplaying = false;
    tplaying = false;
    pplaying = false;
}

void SensorControl::AddClient(int _Socket)
{
    SensorControlClient newclient(_Socket);
    clients.push_back(newclient);
}

void SensorControl::DisconnectClient(int _Socket)
{
    for(unsigned int i = 0;i < clients.size(); i++)
    {
        if(clients[i].socket == _Socket)
        {
            SensorControl::SetPlaying(_Socket,false,false,false);
            clients[i].killthread = true;
        }
    }
}

void SensorControl::SetPlaying(int _Socket, bool _Oxygen, bool _Temperature, bool _Pressure)
{
    //set individual socket state t/f
    for(unsigned int i = 0;i < clients.size();i++)
    {
        if(clients[i].socket == _Socket)
        {
            clients[i].oplaying = _Oxygen;
            clients[i].tplaying = _Temperature;
            clients[i].pplaying = _Pressure;
        }
    }
    //Set playing states
    bool ocheck = false;
    bool tcheck = false;
    bool pcheck = false;
    for(unsigned int i = 0;i < clients.size();i++)
    {
        if(clients[i].oplaying)
        {
            ocheck = true;
        }
        if(clients[i].pplaying)
        {
            pcheck = true;
        }
        if(clients[i].tplaying)
        {
            tcheck = true;
        }
    } 
    oplaying = ocheck;
    tplaying = tcheck;
    pplaying = pcheck;
}

SensorControlClient *SensorControl::GetClient(int _Socket)
{
    for(unsigned int i = 0; i < clients.size();i++)
    {
        if(clients[i].socket == _Socket)
        {
            return &clients[i];
        }
    }
    return NULL;
}