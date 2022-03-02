#include "UnixSocket.h"
#include "net/UnixPacket.h"


namespace CGameEngine
{
    /// UnixSocket public functions ////////////////////////////////////////////////////////////////

    bool UnixSocket::sendData(unsigned char* packet_data, uint32_t packet_size)
    {
        // check for faults
        if(m_fd == -1) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::sendData()", "No fd set!"); return false; }
        //else if(m_rfd == -1) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::sendData()", "No remote fd set!"); return false; }
        else if(!packet_data) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::sendData()", "packet_data pointer is null!"); return false; }
        else if(packet_size == 0 || packet_size > UNIX_PACKET_MAX_SIZE) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::sendData()", "packet_size is incorrect! Tried {}, but max is {}.", packet_size, UNIX_PACKET_MAX_SIZE); return false; }

        int sent = sendto(m_fd, packet_data, packet_size, 0, (struct sockaddr*)&m_remoteAddr, sizeof(m_remoteAddr)); // write out
        if(sent <= 0) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::sendData()", "Failed write() to m_fd ({})!", m_fd); return false; }
        else if(sent != packet_size) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::sendData()", "Failed to write() full packet. Sent only {} of {}!", sent, packet_size); return false; }
        else // success
        {
            memset(packet_data, 0, packet_size); // cleanup
            return true;
        }
    }

    int UnixSocket::receive(unsigned char* packet_data)
    {
        if(m_fd == -1)
        {
           Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::receive()", "No fd set!");
            return -1;
        }

        // read data in
        int bytes = -1;
        socklen_t addrsize = 0;
        bytes = recvfrom(m_fd, packet_data, UNIX_PACKET_MAX_SIZE, 0, (struct sockaddr*)&m_remoteAddr, &addrsize);

        if(bytes > 0)
        {
            /// \TODO: Some logging here
        }

        return bytes;
    }

    void UnixSocket::closeSocket()
    {
        //flush errors, pre closing
        //int err = 1; socklen_t len = sizeof(err);
        //getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
        #if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_LINUX
            unlink(m_sockName.c_str());
            close(m_fd);
        #endif

        m_sockName = "";
        m_fd = -1;
        m_isConnected = false;
    }

    void UnixSocket::setRemoteAddr(std::string& sockName)
    {
        memset(&m_remoteAddr, 0, sizeof(m_remoteAddr));
        m_remoteAddr.sun_family = AF_UNIX;
        std::string fullName = "/tmp/" + m_sockName; // prefix with socket string with '/tmp/'
        strcpy(m_remoteAddr.sun_path, fullName.c_str());
        socklen_t addrLength = strlen(m_remoteAddr.sun_path) + sizeof(m_remoteAddr.sun_family);
    }

    /// UnixSocket private functions //////////////////////////////////////////

    bool UnixSocket::open(bool readOnly)
    {
        if(m_sockName == "" && m_fd == -1)
        {
           Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::open()", "Failure as m_sockName and m_fd are empty!");
            return false;
        }

        // create initial socket
        m_fd = socket(AF_UNIX, SOCK_DGRAM /*| SOCK_CLOEXEC*/, 0);
        if(m_fd == -1)
        {
           Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::open()", "Failed on socket creation, returned {}.", m_fd);
            closeSocket();
            return false;
        }

        // prepare socket
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        memset(&m_remoteAddr, 0, sizeof(m_remoteAddr));
        addr.sun_family = AF_UNIX;
        std::string fullName = "/tmp/" + m_sockName; // prefix with socket string with '/tmp/'
        strcpy(addr.sun_path, fullName.c_str());
        socklen_t addrLength = strlen(addr.sun_path) + sizeof(addr.sun_family);

        /// opening a new socket
        if(readOnly)
        {
            // ensure it wasn't left open
            unlink(addr.sun_path);

            // binding socket
            int bindVal = bind(m_fd, (struct sockaddr*)&addr, addrLength);
            if(bindVal == -1)
            {
               Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::open()", "Failed on bind, returned {}.", bindVal);
                closeSocket();
                return false;
            }

            /*// socket cannot listen for whatever reason
            if(listen(m_fd, SOMAXCONN) == -1)
            {
               Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::open()", "Failed to set listening for UNIX socket!");
                closeSocket();
                return false;
            }*/
        }
        /// connecting to an existing socket
        else
        {
            int retVal = connect(m_fd, (struct sockaddr*)&addr, addrLength);
            if(retVal == -1)
            {
               Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::open()", "Failed to connect UNIX socket ('{}')!", fullName);
                closeSocket();
                return false;
            }
            else { m_remoteAddr = addr; }
        }

        // set non-blocking
        int nonBlocking = 1;
        if(fcntl(m_fd, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
        {
            /// this is performed as a separate step to clarify failures (opening vs non-blocking)
           Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixSocket::open()", "Failed to set non-blocking!");
            closeSocket();
            return false;
        }

        // set as unix socket
        m_type = SocketType::UnixSocket;

       Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "UnixSocket::open()", "Opened Unix Socket on '\033[1m{}\033[0m' (fd {}).", fullName, m_fd);

        // port opened
        return true;
    }
}
