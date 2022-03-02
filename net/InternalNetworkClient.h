#ifndef INTNETWORKCLIENT_H
#define INTNETWORKCLIENT_H

#include "net/NetworkClient.h"

namespace CGameEngine
{

    class InternalNetworkClient : public NetworkClient
    {
        public:
            InternalNetworkClient(SoftwareVersion* swv, uint16_t srcPort, std::string srcHostname, SafeQueue<Datagram*>* dgbuff, uint16_t dstPort, std::string dstHostname = "");
            ~InternalNetworkClient() {}

            bool isPacketValid(Packet* p, sockaddr_storage* sender = nullptr) override { /*return (!p->isDamaged());*/ return true; }
    };
}

#endif // INTNETWORKCLIENT_H
