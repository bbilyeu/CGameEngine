#ifndef INTNETWORKSERVER_H
#define INTNETWORKSERVER_H

#include "net/NetworkServer.h"

namespace CGameEngine
{
    class InternalNetworkServer : public NetworkServer
    {
        public:
            InternalNetworkServer(SoftwareVersion* swv, uint16_t srcPort, SafeQueue<Datagram*>* dgbuff, std::string hostname = "");
            ~InternalNetworkServer() {}

            bool isPacketValid(Packet* p, sockaddr_storage* sender = nullptr) override { /*return (!p->isDamaged());*/ return true; }
    };
}

#endif // INTNETWORKSERVER_H
