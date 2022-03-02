#ifndef SOCKET_H
#define SOCKET_H

#include "common/types.h"
#include "net/net_util.h"

#define TCP_BACKLOG 5

#if PLATFORM == PLATFORM_WINDOWS
    typedef int socklen_t;
#endif

/*
    Ref:
        http://pubs.opengroup.org/onlinepubs/009695399/functions/setsockopt.html
            setsockopt : READ THIS
        http://man7.org/linux/man-pages/man2/setsockopt.2.html
            getsockopt : READ THIS
*/

/// \TODO: Convert "Handle" references to "fd" (file descriptor)
/// \NOTE: TCP IS ONLY SETUP FOR POINT-TO-POINT CONNECTIONS!

namespace SocketType { enum FORMS { NONE, NetSocket, UnixSocket, END }; }

namespace CGameEngine
{
    class Socket
    {
        public:
            bool isOpen() { return (m_fd != -1); }
            int& getFD() { return m_fd; } // correct notation

        protected:
            int m_fd = -1;
            int m_type = SocketType::NONE;
    };
}

#endif // SOCKET_H
