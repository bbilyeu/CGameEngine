#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include "net/Network.h"
#include "net/UnixSocket.h"

namespace CGameEngine
{
//    class NetworkPeer;

    class NetworkServer : public Network
    {
        public:
            NetworkServer() {}
            NetworkServer(SoftwareVersion* swv, uint16_t port, SafeQueue<Datagram*>* dgbuff, std::string hostname = "");
            ~NetworkServer();
            //void listenLoop() override;
            void updateLoop() override;
            bool shutdownSockets() override;
            bool isPacketValid(Packet* p, sockaddr_storage* sender = nullptr) override { return (/*!p->isDamaged() &&*/ p->matchesVersion(m_version)); }
            const unsigned int getConnectionCount() const { return m_netConnections.size(); }
//            void addPeer(NetworkPeer* peer);
//            void changeBuffer(NetConnection* nc, NetworkPeer* np);

            const uint16_t getRequestConnectionOPCode() const { return m_reqConnOP; }
            const uint16_t getAcceptConnectionOPCode() const { return m_acceptConnOP; }
            const uint16_t getDenyConnectionOPCode() const { return m_denyConnOP; }
            void setRequestConnectionOPCode(const uint16_t& opCode) { m_reqConnOP = opCode; }
            void setAcceptConnectionOPCode(const uint16_t& opCode) { m_acceptConnOP = opCode; }
            void setDenyConnectionOPCode(const uint16_t& opCode) { m_denyConnOP = opCode; }

        protected:
            SafeUnorderedMap<std::string, NetConnection*> m_netConnections; // clients
//            SafeUnorderedMap<uint32_t, NetworkPeer*> m_internalConnections; // zones, services, etc
            SafeUnorderedMap<std::string, uint32_t> m_closedConnections; // IPStr and time to remove
            uint16_t m_reqConnOP = OP_ConnectionRequest;
            uint16_t m_acceptConnOP = OP_ConnectionAccepted;
            uint16_t m_denyConnOP = OP_ConnectionDisconnect;
    };
}

#endif // NETWORKSERVER_H
