//#ifndef NETWORKPEER_H
//#define NETWORKPEER_H
//
//#include "net/Network.h"
//
///// \TODO: Use eventfd() to alert the other side of pending packets
//
///// \DO
///// \NOT
///// \USE
//
//namespace CGameEngine
//{
//    class NetworkPeer : public Network
//    {
//        public:
//            NetworkPeer(SoftwareVersion* swv, SafeQueue<Datagram*>* dgbuff, uint16_t port, bool isSideB = false);
//            virtual ~NetworkPeer();
//            void loop();
//            bool shutdownSockets() override;
//            bool isPacketValid(Packet* p, sockaddr_storage* sender = nullptr) override { /*return (!p->isDamaged());*/ return true; }
//            void send(uint16_t opCode, unsigned char** data, uint32_t dataLength) { Network::send(nullptr, opCode, data, dataLength); }
//            void sendBuiltin(uint16_t opCode, uint32_t seqID = 0, uint32_t pktNum = 0) { Network::sendBuiltin(nullptr, opCode, seqID, pktNum); }
//            void sendDatagram(Datagram** d);
//            void sendSimple(uint16_t opCode) { Network::sendSimple(nullptr, opCode); }
//            void sendRetryResponse(Packet* p) { Network::sendRetryResponse(nullptr, p); }
//            static void startLoop(NetworkPeer* n) { n->loop(); }
//            const bool& isConnected() const { return m_isConnected; }
//            const uint16_t& getDstPort() const { return m_dstPort; }
//            addrinfo* getDstAddress() { return m_dstAddress; }
//            SafeQueue<Datagram*>* getSendDGQueue() { return &m_sendDatagramQueue; }
//
//            // dead end overrides
//            void listenLoop() override {}
//            void updateLoop() override {}
//
//            /// \DO
//            /// \NOT
//            /// \USE
//
//        private:
//            bool m_isSideB = false; // determines if accept() or connect() is used
//            uint16_t m_dstPort = 0;
//            // m_srcPort is declared in Network()
//            NetConnection* m_peer = nullptr; // connection data FROM peer
//            addrinfo* m_dstAddress = nullptr; // remote peer's addrinfo
//            SafeQueue<Datagram*> m_sendDatagramQueue;
//    };
//}
//
///// \DO
///// \NOT
///// \USE
//
//#endif // NETWORKPEER_H
