#ifndef NETSOCKET_H
#define NETSOCKET_H

#include "net/Socket.h"

namespace CGameEngine
{
    class NetSocket : public Socket
    {
        /// \TODO: Break TCP out into it's own setup
        public:
            NetSocket() {}
            NetSocket(addrinfo* addr, bool tcp = false, bool noBind = false) : m_isTCP(tcp) { open(addr, noBind); }
            ~NetSocket() { closeSocket(); }
            void closeSocket();
            //bool isConnected();
            //bool tryAcceptConnection();
            //bool tryConnect(const sockaddr_storage* dest);
            bool sendData(const sockaddr_storage* destination, unsigned char* packet_data, uint32_t packet_size);
            bool broadcastSend(const sockaddr_storage* destination, unsigned char* packet_data, uint32_t packet_size);
            int receive(struct sockaddr_storage* sender, unsigned char* buffer, int fd = -1);
            const int& getRemoteFD() const { return m_remoteFD; }

        private:
            bool open(addrinfo* addr, bool noBind = false);
            bool setBroadcast(bool val);

            // TCP SPECIFIC
            bool m_isTCP = false;
            bool m_hasAccepted = false;
            bool m_hasConnected = false;
            int m_remoteFD = -1;
    };
}

#endif // NETSOCKET_H
