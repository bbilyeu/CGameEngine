#ifndef UNIXSOCKET_H
#define UNIXSOCKET_H

#include "net/Socket.h"

/// \TODO: Handle remote end disconnecting

/*
    Ref:
        https://dvdhrm.wordpress.com/2015/06/20/from-af_unix-to-kdbus/

*/

namespace CGameEngine
{
    class UnixSocket : public Socket
    {
        public:
            UnixSocket() {}
            UnixSocket(std::string& sockName, bool readOnly = true) : m_sockName(sockName) { open(readOnly); }
            ~UnixSocket() { closeSocket(); }
            void closeSocket();
            void setRemoteAddr(std::string& sockName);
            bool sendData(unsigned char* packet_data, uint32_t packet_size);
            int receive(unsigned char* buffer);
            const bool isConnected() const { return true; }
            const std::string& getSocketName() const { return m_sockName; }

        private:
            bool m_isConnected = false;
            struct sockaddr_un m_remoteAddr;
            std::string m_sockName = "";
            bool open(bool readOnly);
    };
}
#endif // UNIXSOCKET_H
