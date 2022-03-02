#ifndef NETWORKIPC_H
#define NETWORKIPC_H

#include "net/Network.h"
#include "net/UnixSocket.h"
#include "net/UnixPacket.h"
#include <iostream>

/*
    Process A  <------>  Process B
    (primary)           (secondary)

    Ref:
        ancillary messaging : https://linux.die.net/man/3/cmsg
            Part 2 : https://stackoverflow.com/questions/28003921/sending-file-descriptor-by-linux-socket/

*/

/// \TODO: This setup has the potential to choke on high "send" count
/// \TODO: Handle remote end disconnecting

namespace CGameEngine
{
    class NetworkIPC
    {
        public:
            NetworkIPC(std::string sockname, Network* net, SafeQueue<UnixPacket*>* packetQueue); // create primary (left) side
            NetworkIPC(std::string sockname, SafeQueue<UnixPacket*>* packetQueue); // create secondary (right) side
            virtual ~NetworkIPC();
            bool shutdownSockets();
            void mainLoop();
            void addDatagram(Datagram** dg); // Network (Process A) -> Process B
            void addEventCV(std::condition_variable& cv) { m_cv = &cv; }
            void completePair();
            void sendSimple(uint16_t opCode, uint32_t sender = 0); // basic OPCode responses
            void send(uint16_t opCode, uint32_t senderID, unsigned char** data, uint16_t dataLength, uint64_t arrival = 0); // Process B -> Network (Process A)
            void stop() { m_isActive = false; }
            void setDisconnected() { m_isConnected = false; } // used when either side "dies"

            const bool& isConnected() const { return m_isConnected; }
            bool isReadConnected();
            bool isWriteConnected();
            const uint32_t& getUniqueID() const { return m_uniqueID; }
            const std::string& getSocketName() const { return m_sockName; }

            // thread starter
            static void startLoop(NetworkIPC* n) { n->mainLoop(); }

        private:
            void generateID();
            bool m_isActive = false;
            bool m_isConnected = false; // both sides connected
            bool m_isPrimary = false; // left side
            uint16_t m_pollTimeout = 5000; // MS
            uint32_t m_uniqueID = 0;
            std::string m_sockName = "";
            UnixSocket* m_readSocket = nullptr;
            UnixSocket* m_writeSocket = nullptr;
            Network* m_net = nullptr; // primary side only!!
            std::thread* m_thread = nullptr;
            std::condition_variable* m_cv = nullptr;

            // this is the outbound queue on Process A (primary)
            // but it is the inbound queue on Process B (secondary), or m_datagramBuffer
            SafeQueue<UnixPacket*>* m_unixPackets = nullptr;
    };
}

#endif // NETWORKIPC_H
