#include "net/InternalNetworkClient.h"

namespace CGameEngine
{
    InternalNetworkClient::InternalNetworkClient(SoftwareVersion* swv, uint16_t srcPort, std::string srcHostname, SafeQueue<Datagram*>* dgbuff, uint16_t dstPort, std::string dstHostname /*= ""*/)
    {
        // check if ports are open
        /// \TODO: DEBUG DISABLED
        //if(!isPortOpen(srcPort)) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "InternalNetworkClient::InternalNetworkClient()", "Port {} is in use!", srcPort); }
        //if(!isPortOpen(dstPort)) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "InternalNetworkClient::InternalNetworkClient()", "Port {} is in use!", dstPort); }

        // set variables
        m_version = swv;
        m_srcPort = srcPort;
        m_datagramBuffer = dgbuff;
        m_dstPort = dstPort;
        m_srcHostname = srcHostname;
        m_dstHostname = dstHostname;
        m_networkType = NetworkType::InternalClient;

        // generate internal source addrinfo
        generateInternalAddress(srcPort, &m_srcAddress, m_srcHostname); // 10.x.x.x subnet
		Logger::getInstance().Log(Logs::DEBUG, "InternalNetworkClient::InternalNetworkClient()", "IP: {}", getIPString((struct sockaddr_storage*)m_srcAddress->ai_addr));

        // open socket
        m_socket = new NetSocket(m_srcAddress);
        if(!m_socket->isOpen()) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "InternalNetworkClient::InternalNetworkClient()", "Port [{}] could not be opened!", m_srcPort); }
        m_socketPairs[m_socket->getFD()] = std::make_pair(m_socket, m_datagramBuffer);

        // generate internal destination addrinfo
        generateInternalAddress(dstPort, &m_dstAddress, dstHostname);

        // set listening active
        m_isActive = true;

        // start thread loops
        m_sendThread = new std::thread(startSendLoop, this); // send processing
        m_listenThread = new std::thread(NetworkClient::startListenLoop, this); // network (tcp/udp) listen thread
        m_updateThread = new std::thread(NetworkClient::startUpdateLoop, this); // all network traffic update loop

       Logger::getInstance().Log(Logs::INFO, Logs::Network, "InternalNetworkClient::InternalNetworkClient()", "Started threads with srcAddress \033[1m{}", getIPString((struct sockaddr_storage*)m_srcAddress->ai_addr));
    }
}
