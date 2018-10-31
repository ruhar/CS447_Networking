#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>

namespace cs447
{
    void Hello();
    void Goodbye();
    void RTSPReceiverClient(int _ServerPort);
}