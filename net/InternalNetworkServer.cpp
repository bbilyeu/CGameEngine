#include "net/InternalNetworkServer.h"

namespace CGameEngine
{
    InternalNetworkServer::InternalNetworkServer(SoftwareVersion* swv, uint16_t srcPort, SafeQueue<Datagram*>* dgbuff, std::string hostname /*= ""*/)
    {
        if(!isPortOpen(srcPort)) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "InternalNetworkServer::InternalNetworkServer()", "Port {} is in use!", srcPort); }

        // set variables
        m_version = swv;
        m_srcPort = srcPort;
        m_datagramBuffer = dgbuff;
        m_networkType = NetworkType::InternalServer;

        // generate internal addrinfo
        generateInternalAddress(m_srcPort, &m_srcAddress, hostname);
		Logger::getInstance().Log(Logs::DEBUG, "InternalNetworkServer::InternalNetworkServer()", "IP: {}", getIPString((struct sockaddr_storage*)m_srcAddress->ai_addr));

        // open socket
        m_socket = new NetSocket(m_srcAddress);
        if(!m_socket->isOpen()) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "InternalNetworkServer::InternalNetworkServer()", "Port [{}] could not be opened!", m_srcPort); }
        m_socketPairs[m_socket->getFD()] = std::make_pair(m_socket, m_datagramBuffer);

        // set listening active
        m_isActive = true;

        // start thread loops
        m_sendThread = new std::thread(startSendLoop, this); // send processing
        m_listenThread = new std::thread(NetworkServer::startListenLoop, this); // network (tcp/udp) listen thread
        m_updateThread = new std::thread(NetworkServer::startUpdateLoop, this); // all network traffic update loop

       Logger::getInstance().Log(Logs::INFO, Logs::Network, "InternalNetworkServer::InternalNetworkServer()", "Started threads with srcAddress \033[1m{}", getIPString((struct sockaddr_storage*)m_srcAddress->ai_addr));
    }
}
