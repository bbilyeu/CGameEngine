//#include "NetworkPeer.h"
//
//#include "common/Timer.h"
//
///// \DO
///// \NOT
///// \USE
//
//namespace CGameEngine
//{
//    NetworkPeer::NetworkPeer(SoftwareVersion* swv, SafeQueue<Datagram*>* dgbuff, uint16_t port, bool isSideB /*= false*/) //: Network(swv, srcPort, dgbuff, "localhost", true)
//        : Network()
//    {
//        if(!initSockets()) { Logger::getInstance().Log(Logs::FATAL, Logs::NetworkPeer, "NetworkPeer::NetworkPeer()", "[Windows] Networking could NOT be started!"); }
//        else
//        {
//            m_srcPort = port;
//            m_isSideB = isSideB;
//            m_isTCP = true;
//            m_version = swv;
//            m_datagramBuffer = dgbuff;
//            m_networkType = NetworkType::Peer;
//            m_pollTimeout = 5000;
//
//            // generate source (listen) address
//            generateAddress("localhost", m_srcPort, &m_srcAddress);
//           Logger::getInstance().Log(Logs::DEBUG, "NetworkPeer::NetworkPeer()", "Source \033[1m{}", getIPString((struct sockaddr_storage*)m_srcAddress->ai_addr));
//
//            // open socket
//            m_socket = new NetSocket(m_srcAddress, m_isTCP, m_isSideB);
//            if(!m_socket->isOpen()) { Logger::getInstance().Log(Logs::FATAL, Logs::NetworkPeer, "NetworkPeer::NetworkPeer()", "Port {} could not be opened!", m_srcPort); }
//
//            // set listening active
//            m_isActive = true;
//            m_netListening = true;
//
//            // launch threads
//            m_listenThread = new std::thread(startLoop, this); // listen and processing
//            m_sendThread = new std::thread(startSendLoop, this); // send processing
//
//           Logger::getInstance().Log(Logs::INFO, Logs::NetworkPeer, "NetworkPeer::NetworkPeer()", "Started threads.");// with srcAddress \033[1m{}", getIPString((struct sockaddr_storage*)m_srcAddress->ai_addr));
//        }
//    }
//
//    NetworkPeer::~NetworkPeer()
//    {
//        while(!shutdownSockets())
//        {
//           Logger::getInstance().Log(Logs::WARN, Logs::NetworkPeer, "NetworkPeer::~NetworkPeer()", "Waiting on shutdownSockets() to close.");
//        }
//    }
//
//    bool NetworkPeer::shutdownSockets()
//    {
//        m_isActive = false;
//        m_netListening = false;
//
//        #if PLATFORM == PLATFORM_WINDOWS
//                WSACleanup();
//        #endif
//
//        if(m_dstAddress) { /*delete m_dstAddress;*/ freeaddrinfo(m_dstAddress); m_dstAddress = nullptr; }
//        m_dstPort = -1;
//        m_srcPort = -1;
//        safeDelete(m_peer);
//        Logger::getInstance().Log(Logs::DEBUG, "NetworkPeer::shutdownSockets()", "Completed Successfully!");
//        return true;
//    }
//
//    void NetworkPeer::loop()
//    {
//        unsigned char* buffer = new unsigned char[PACKET_MAX_SIZE];
//        struct sockaddr_storage sender;
//        int bytes_read = 0;
//        int retVal = 0;
//
//        // poll()
//        struct pollfd ufds[1];
//        ufds[0].fd = m_socket->getFD();
//        ufds[0].events = POLLIN | POLLPRI;
//
//        Timer acceptWaitTimer = Timer("acceptWait", m_pollTimeout*300);
//
//        while(m_isActive)
//        {
//            // if not accepting connections yet, keep skipping
//            if(!m_netListening) { continue; }
//            // do not process packets or attempt to until we have accepted a connection
//            else if(!m_isConnected)
//            {
//                if(m_isSideB) // sideB : connect()
//                {
//                    if(!m_socket->tryConnect((struct sockaddr_storage*)m_srcAddress->ai_addr))
//                    {
//                        if(acceptWaitTimer.isExpired())
//                        {
//                           Logger::getInstance().Log(Logs::WARN, Logs::NetworkPeer, "NetworkPeer::loop()", "Timed out waiting for connection. Closing!");
//                            m_isActive = false;
//                        } else { continue; }
//                    } else { m_isConnected = true; }
//                   Logger::getInstance().Log(Logs::VERBOSE, Logs::NetworkPeer, "NetworkPeer::loop()", "tryConnection() returned [{}]", m_isConnected);
//                }
//                else // sideA : accept()
//                {
//                    if(!m_socket->tryAcceptConnection())
//                    {
//                        if(acceptWaitTimer.isExpired())
//                        {
//                           Logger::getInstance().Log(Logs::WARN, Logs::NetworkPeer, "NetworkPeer::loop()", "Timed out waiting for connection. Closing!");
//                            m_isActive = false;
//                        } else { continue; }
//                    }
//                    else { m_isConnected = true; ufds[0].fd = m_socket->getFD(); }
//                   Logger::getInstance().Log(Logs::VERBOSE, Logs::NetworkPeer, "NetworkPeer::loop()", "tryAcceptConnection() returned [{}]", m_isConnected);
//                }
//            }
//
//            // clear buffer and bytes_read, setup sender variable
//            memset(buffer, 0, PACKET_MAX_SIZE);
//            memset(&sender, 0, sizeof(struct sockaddr_storage));
//            bytes_read = 0;
//            retVal = 0;
//
//            // actually poll for data
//            retVal = poll(ufds, 2, m_pollTimeout); // poll all sockets
//
//            if(retVal < 0) { Logger::getInstance().Log(Logs::CRIT, Logs::NetworkPeer, "NetworkPeer::loop()", "poll() returned [{}], this is likely fatal. Stopping loop", retVal); m_isActive = false; }
//            //else if(retVal == 0) { Logger::getInstance().Log(Logs::WARN, Logs::NetworkPeer, "NetworkPeer::loop()", "poll() timed out without data. This may be bad."); m_isActive = false; } /// \TODO: Is this important or even accurate?
//            else if(retVal > 0)
//            {
//                // read in data
//                if(ufds[0].revents & POLLIN) { bytes_read = m_socket->receive(&sender, buffer, ufds[0].fd); }
//                else if(ufds[1].revents & POLLIN) { bytes_read = m_socket->receive(&sender, buffer, ufds[0].fd); }
//                if(bytes_read <= PACKET_MIN_RCV_SIZE) { continue; } // catch too small data and abandon
//                if(!m_peer) { std::string ipStr = getIPString(&sender); m_peer = new NetConnection((Network*)this, &sender, ipStr, 1, m_datagramBuffer); }
//
//                //Logger::getInstance().Log(Logs::VERBOSE, Logs::NetworkPeer, "NetworkPeer::loop()", "--- NetworkPeer::loop: Bytes Read [{}] ---", bytes_read);
//                Packet* p = new Packet(buffer, bytes_read, Time::getInstance().nowMS());
//
//                // packet arrived from expected source and isn't broken
//                if(isPacketValid(p))
//                {
//                        /// \TODO: FLESH THIS OUT
//                        switch(p->op_code)
//                        {
//                            case OP_RetransmissionAck:
//                            {
//                                /// \TODO: Destroy sequence within connection
//                               Logger::getInstance().Log(Logs::DEBUG, "NetworkPeer::loop()", "Received retransmissions ACK! seqID [{}], pktNum [{}]", p->seqIdent, p->pktNum);
//                                m_peer->addPacket(&p);
//                                break;
//                            }
//                            case OP_RetransmissionImpossible:
//                            {
//                                /// \TODO: Destroy sequence within connection
//                               Logger::getInstance().Log(Logs::DEBUG, "NetworkPeer::loop()", "Received retransmissions impossible! seqID [{}], pktNum [{}]", p->seqIdent, p->pktNum);
//                                m_peer->addPacket(&p);
//                                break;
//                            }
//                            case OP_ConnectionDisconnect:
//                            {
//                                /// \TODO: Create way to handle server disconnecting this client
//                               Logger::getInstance().Log(Logs::CRIT, Logs::NetworkPeer, "NetworkPeer::loop()", "Closing connection to server!");
//                                m_peer->setClosed(true);
//                                break;
//                            }
//                            case OP_Ack: // sequence has completed or destroyed, let sender know
//                            {
//                               Logger::getInstance().Log(Logs::DEBUG, "NetworkPeer::loop()", "Received ACK for seqID [{}].", p->seqIdent);
//                                m_peer->addPacket(&p);
//                                break;
//                            }
//                            default: // normal packets
//                            {
//                                m_peer->addPacket(&p); // add packet to connection
//                                break;
//                            }
//                        }
//                }
//                else
//                {
//                   Logger::getInstance().Log(Logs::DEBUG, "NetworkPeer::loop()", "Packet damaged or from unknown source [{}].", getIPString(&sender));
//                }
//
//                // cleanup, if not handed off
//                safeDelete(p);
//            }
//        }
//
//        safeDeleteArray(buffer);
//       Logger::getInstance().Log(Logs::INFO, Logs::NetworkPeer, "NetworkPeer::loop()", "loop exited!");
//        m_netListening = false;
//        m_isActive = false;
//        m_socket->closeSocket();
//    }
//
//    void NetworkPeer::sendDatagram(Datagram** d)
//    {
//        // take ownership
//        Datagram* dg = (*d);
//        *d = nullptr;
//
//        if(!dg) { Logger::getInstance().Log(Logs::CRIT, Logs::NetworkPeer, "NetworkPeer::sendDatagram()", "Blank Datagram pointer passed!"); }
//        else { Network::send(nullptr, dg->op_code, &dg->data, dg->dataLength); m_sendCV.notify_one(); }
//
//        // cleanup
//        safeDelete(dg);
//    }
//}
//
///// \DO
///// \NOT
///// \USE
