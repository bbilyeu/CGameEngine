#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include "net/Network.h"

/// \TODO: Evaluate holding all packets when server communication lost or not yet accepted

namespace CGameEngine
{
    class NetworkClient : public Network
    {
        public:
            NetworkClient() {}
            NetworkClient(SoftwareVersion* swv, uint16_t port, SafeQueue<Datagram*>* dgbuff, std::string dstHostname, uint16_t dstPort);
            NetworkClient(SoftwareVersion* swv, uint16_t port, SafeQueue<Datagram*>* dgbuff);
            virtual ~NetworkClient();
            //void listenLoop() override;
            void updateLoop() override;
            bool shutdownSockets() override;
            bool isPacketValid(Packet* p, sockaddr_storage* sender = nullptr) override;
            //void setDestination(std::string host, uint16_t port) { m_dstPort = port; generateAddress(host, m_dstPort, &m_dstAddress); }
            void send(uint16_t opCode, unsigned char** data, uint32_t dataLength) { Network::send((struct sockaddr_storage*)m_dstAddress->ai_addr, opCode, data, dataLength); }
            void sendBuiltin(uint16_t opCode, uint32_t seqID = 0, uint32_t pktNum = 0) { Network::sendBuiltin((struct sockaddr_storage*)m_dstAddress->ai_addr, opCode, seqID, pktNum); }
            void sendSimple(uint16_t opCode) { Network::sendSimple((struct sockaddr_storage*)m_dstAddress->ai_addr, opCode); }
            void sendRetryResponse(Packet* p) { Network::sendRetryResponse((struct sockaddr_storage*)m_dstAddress->ai_addr, p); }
            static void startUpdateLoop(NetworkClient* n) { n->updateLoop(); }
            static void startNetListen(NetworkClient* n) { n->listenLoop(); }
            addrinfo* getDstAddress() { return m_dstAddress; }
            sockaddr_storage* getDstSock() { return (struct sockaddr_storage*)m_dstAddress->ai_addr; }

        protected:
            bool m_isConnectionAccepted = false; // client only, signify server accepted
            uint16_t m_dstPort = -1; // needed?
            std::string m_dstHostname = "";
            NetConnection* m_serverConnection = nullptr;
            addrinfo* m_dstAddress = nullptr; // server's addrinfo

            uint32_t MIN_SEQ_ID = 2000000001;
            uint32_t MAX_SEQ_ID = 4000000000;
    };
}

#endif // NETWORKCLIENT_H
