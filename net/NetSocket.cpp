#include "NetSocket.h"
#include "net/Packet.h"


namespace CGameEngine
{
    /// NetSocket ////////////////////////////////////////////////////////////////

    /// START TCP ONLY! /////
    /*bool NetSocket::isConnected()
    {
        if(m_fd <= 0 || !m_isTCP) { return false; }

        if(m_hasAccepted || m_hasConnected) { return true; }
        else { return false; }
    }

    bool NetSocket::tryAcceptConnection()
    {
        // Run on Peer-A
        // exit if not tcp or tcp & remoteFD already working
        if(m_fd <= 0 || !m_isTCP || m_hasAccepted) { return false; }

        struct sockaddr remoteConnection;
        socklen_t fromLen = sizeof(struct sockaddr);
        int retVal = accept(m_fd, &remoteConnection, &fromLen);

        if(retVal < 0) { m_hasAccepted = false; }
        else
        {
            //std::cout<<"tryAcceptConnection accepted remoteFD("<<retVal<<")\n";
            m_hasAccepted = true;

            // close old socket
            int err = 1; socklen_t len = sizeof(err);
            getsockopt(m_fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
            #if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_LINUX
                shutdown(m_fd, SHUT_RDWR);
                close(m_fd);
            #elif PLATFORM == PLATFORM_WINDOWS
                closesocket(m_fd);
            #endif

            // set new one as "main"
            m_fd = retVal;
        }
        return m_hasAccepted;
    }

    bool NetSocket::tryConnect(const sockaddr_storage* dest)
    {
        // Run on Peer-B
        // exit if not TCP, no passed sockaddr, or already connected
        if(m_fd <= 0 || !m_isTCP || m_hasConnected || !dest) { return false; }

        socklen_t fromLen = sizeof(struct sockaddr_storage);
        int retVal = connect(m_fd, (struct sockaddr*)dest, fromLen);

        if(retVal < 0) { m_hasConnected = false; }
        else { std::cout<<"tryConnect connected to remoteFD("<<m_remoteFD<<")\n"; m_hasConnected = true; }
        return m_hasConnected;
    }*/
    /// END TCP ONLY! /////

    bool NetSocket::sendData(const sockaddr_storage* destination, unsigned char* packet_data, uint32_t packet_size)
    {
        if(m_fd <= 0 || !packet_data || packet_size <= 0 || (!destination && !m_isTCP)) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetSocket::send()" ,"Generic error"); return false; }

        // send, recording bytes sent
        int sent_bytes = 0;
        if(m_isTCP) { sent_bytes = send(m_fd, packet_data, packet_size, 0); }
        else { sent_bytes = sendto(m_fd, packet_data, packet_size, 0, (struct sockaddr*)destination, sizeof(sockaddr_in)); }

        // cleanup
        memset(packet_data, 0, packet_size);

        if(sent_bytes != packet_size)
        {
           Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetSocket::send()", "Failed to send packet! Sent [{}] of [{}].", sent_bytes, packet_size);
            return false;
        }

        //Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "NetSocket::send()", "Packet Size [{}], Sent Bytes [{}]", packet_size, sent_bytes);
        return true;
    }

    bool NetSocket::broadcastSend(const sockaddr_storage* destination, unsigned char* packet_data, uint32_t packet_size)
    {
        // enable broadcast
        if(!setBroadcast(true)) { return false; }

        bool retVal = sendData(destination, packet_data, packet_size);

        // disable broadcast
        if(!setBroadcast(false))
        {
           Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetSocket::broadcastSend()", "Failed to disable broadcast opt. Trying again...");
            if(!setBroadcast(false)) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "NetSocket::broadcastSend()", "Failed to disable broadcast opt again. Serious issue with socket."); }
        }

        // return outcome
        return retVal;
    }

    int NetSocket::receive(struct sockaddr_storage* sender, unsigned char* buffer, int fd /*= -1*/)
    {
        socklen_t fromLen = sizeof(struct sockaddr_storage);
        int bytes = -1;
        if(fd <= 0) { fd = m_fd; }

        // actual pickup from network device
        if(m_isTCP) { bytes = recv(fd, buffer, PACKET_MAX_SIZE-1, 0); }
        else { bytes = recvfrom(fd, buffer, PACKET_MAX_SIZE, 0, (struct sockaddr*)sender, &fromLen); }

        if(bytes > 0)
        {
            //std::cout<<"NetSocket::receive: Pre-termination ["<<buffer<<"]\n";
            // terminate end of packet
            //buffer[PACKET_MAX_SIZE] = 0;
            //std::cout<<"NetSocket::receive: Post-termination ["<<buffer<<"]\n";

            // DEBUG
            /*try
            {
                std::cout<<"IP:\t"<<ntohl(sender->sin_addr.s_addr)<<"\n";
                std::cout<<"Port:\t"<<ntohs(sender->sin_port)<<"\n";
                std::cout<<"Data:\t"<<packet_data<<"\n";
            }
            catch (...)
            {
                printf("Couldn't get IP!\n");
            }*/
            //std::cout<<"NetSocket::receive(sockaddr_storage*, unsigned char*) : return ["<<bytes<<"]\n";
            //Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "NetSocket::receive()", "Received \033[92m[{}]\033[0m bytes", bytes);
        }
        //else if(bytes == -1) { bytes = 0; } // catch to prevent rollover as we're using uint32

        /// \TODO : implement diag info for logging
        //std::cout<<"NetSocket::receive size ["<<sizeof(packet_data)<<"] (post)\n";
        //unsigned int from_address = ntohl(from.sin_addr.s_addr);
        //unsigned int from_srcPort = ntohs(from.sin_port);
        //sender = Address(from_address, from_srcPort);
        //sender = (struct addrinfo*)&from;
        //sender->ai_addr = (struct sockaddr*)&from;
        //sender = inet_ntoa(from.sin_addr);
        return bytes;
    }

    void NetSocket::closeSocket()
    {
        //flush errors, pre closing
        int arr[2] = { m_fd, m_remoteFD };
        for(int i = 0; i < 2; i++)
        {
            if(arr[i] != -1)
            {
                int err = 1; socklen_t len = sizeof(err);
                getsockopt(arr[i], SOL_SOCKET, SO_ERROR, (char*)&err, &len);
                #if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_LINUX
                    shutdown(arr[i], SHUT_RDWR);
                    close(arr[i]);
                #elif PLATFORM == PLATFORM_WINDOWS
                    closesocket(arr[i]);
                #endif
                arr[i] = -1;
            }
        }

        m_isTCP = false;
        m_hasAccepted = false;
    }

    /// NetSocket private functions ///////////////////////////////////////////

    bool NetSocket::open(addrinfo* addr, bool noBind /*= false*/)
    {
		Logger::getInstance().Log(Logs::DEBUG, "NetSocket::open()", "Starting NetSocket open");

        // catch nullptr
        if(!addr) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetSocket::open()", "Passed nullptr for addr!\n"); return false; }

        // handle for socket
        m_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(m_fd <= 0)
        {
           Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetSocket::open()", "Failed to create socket! m_fd [{}]", m_fd);
            return false;
        }

        int bindVal = -1;

        // if UDP or TCP-server
        if(!m_isTCP || (m_isTCP && !noBind))
        {
            if(bindVal = bind(m_fd, addr->ai_addr, addr->ai_addrlen) < 0)
            {
               Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetSocket::open()", "Failed to bind socket, returned [{}]", bindVal);
                closeSocket();
                m_fd = -1; // failed
                return false;
            }

            // set non-blocking mode
            #if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_LINUX
                int nonBlocking = 1;
                if(fcntl(m_fd, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
                {
                   Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetSocket::open()", "[Linux] Failed to set non-blocking!");
                    return false;
                }
            #elif PLATFORM == PLATFORM_WINDOWS
                DWORD nonBlocking = 1;
                if(ioctlsocket(m_fd, FIONBIO, &nonBlocking) != 0)
                {
                   Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetSocket::open()", "[Windows] Failed to set non-blocking!");
                    return false;
                }
            #endif

            /*if(m_isTCP)
            {
                if(listen(m_fd, TCP_BACKLOG) == -1) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "NetSocket::open()", "Failed calling TCP listen()"); }
            }*/
        }

        // set as network socket
        m_type = SocketType::NetSocket;

        // debug logging
        std::string debug(sizeof(INET_ADDRSTRLEN+1), '\0');
        std::string proto = (m_isTCP) ? "TCP" : "UDP";
		Logger::getInstance().Log(Logs::DEBUG, "NetSocket::open()", "Opened \033[1m{}\033[0m socket on {}", proto, inet_ntop(AF_INET, &((struct sockaddr_in*)((struct sockaddr_storage*)addr->ai_addr))->sin_addr, &debug[0], INET_ADDRSTRLEN));

        // port opened
        return true;
    }

    bool NetSocket::setBroadcast(bool val)
    {
        int opt = (val) ? 1 : 0; socklen_t len = sizeof(opt);
        int err = setsockopt(m_fd, SOL_SOCKET, SO_BROADCAST, &opt, len);
        if(err == -1) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetSocket::setBroadcast()", "Setting SO_BROADCAST to NetSocket handle [{}] failed!", m_fd); return false; }
        else { return true; }
    }
}
