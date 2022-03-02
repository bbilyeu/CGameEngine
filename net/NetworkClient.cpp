#include "net/NetworkClient.h"


namespace CGameEngine
{
    /// NetworkClient ///////////////////////////////////////////////////////////////

    NetworkClient::NetworkClient(SoftwareVersion* swv, uint16_t port, SafeQueue<Datagram*>* dbbuff, std::string dstHostname, uint16_t dstPort) : Network(swv, port, dbbuff)
    {
        if(!m_isActive) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "NetworkClient::NetworkClient()", "NetworkClient failed to start using Network() base constructor!"); }
        else
        {
            m_dstHostname = dstHostname;
            m_dstPort = dstPort;
            generateAddress(dstHostname, m_dstPort, &m_dstAddress);
            m_sendThread = new std::thread(startSendLoop, this); // send processing
            m_listenThread = new std::thread(startListenLoop, this); // network (tcp/udp) listen thread
            m_updateThread = new std::thread(startUpdateLoop, this); // all network traffic update loop
            m_networkType = NetworkType::Client;

           Logger::getInstance().Log(Logs::INFO, Logs::Network, "NetworkClient::NetworkClient()", "Started threads with srcAddress \033[1m{}", getIPString((struct sockaddr_storage*)m_srcAddress->ai_addr));
        }
    }

    NetworkClient::NetworkClient(SoftwareVersion* swv, uint16_t port, SafeQueue<Datagram*>* dbbuff) :
        Network(swv, port, dbbuff)
    {
        if(!m_isActive) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "NetworkClient::NetworkClient()", "'Thin' NetworkClient ctor failed to start using Network() base constructor!"); }
    }

    NetworkClient::~NetworkClient()
    {
        while(!shutdownSockets())
        {
           Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkClient::~NetworkClient()", "Waiting on shutdownSockets() to close.");
        }
    }

    bool NetworkClient::shutdownSockets()
    {
        m_isActive = false;
        m_netListening = false;
        m_sendQueue.clear();
        m_packetBuffer.clear();
        while(!m_packetBuffer.empty()) { m_packetBuffer.front().destroy(); m_packetBuffer.pop(); }
        if(m_dstAddress) { /*delete m_dstAddress;*/ freeaddrinfo(m_dstAddress); m_dstAddress = nullptr; }
        m_dstPort = -1;
        m_dstHostname = "";
        safeDelete(m_serverConnection);
       Logger::getInstance().Log(Logs::DEBUG, "NetworkClient::shutdownSockets()", "Completed Successfully!");
        return true;
    }

    bool NetworkClient::isPacketValid(Packet* p, sockaddr_storage* sender /*= nullptr*/)
    {
        bool retVal = (p->matchesVersion(m_version) && sender);// && isSameSource(sender, m_dstAddress));
        return retVal;
    }

    void NetworkClient::updateLoop()
    {
        std::queue<std::string> toBeClosed;
        std::mutex ulmutex;
        std::unique_lock<std::mutex> updateLock(ulmutex);

        while(m_isActive)
        {
            if(m_packetBuffer.empty()) { m_updateCV.wait(updateLock); }

            int pkts = m_packetBuffer.size();
            while(pkts > 0)
            {
                PacketPair* pp = &m_packetBuffer.front();
                Packet* p = pp->pkt;
                sockaddr_storage* sender = &pp->sas;
                uint32_t pktCRC = 0;

                // catch line
                if(!p || !sender) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkClient::updateLoop()", "Broken PacketPair!"); /*p = nullptr; sender = nullptr;*/}
                else
                {
                    bool broken = true;

                    // check identifier
                    try { broken = memcmp(p->identifier, m_identifier.c_str(), IDENT_SIZE); }
                    catch (...) { broken = true; }

                    if(broken)
                    {
                       Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkClient::updateLoop()", "Packet did NOT meet identifier! [{}] ({} remaining)", m_identifier, pkts);
                    }
                    else
                    {
                        // if connection has not been "accepted" yet
                        if(!m_isConnectionAccepted)
                        {
                            if(p->op_code == OP_ConnectionAccepted)
                            {
                               Logger::getInstance().Log(Logs::INFO, Logs::Network, "NetworkClient::updateLoop()", "Connection accepted by server.");
                                m_isConnectionAccepted = true;
                                std::string ipStr = getIPString(sender);
                                m_serverConnection = new NetConnection(this, sender, ipStr, 1, m_datagramBuffer);
                                m_serverConnection->addPacket(&p);
                            }
                            else
                            {
                               Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "NetworkClient::updateLoop()", "Connection is not accepted, yet packet with OP Code [{}] arrived and was discarded. Packet Validity [{}], sender [{}]", p->op_code, false, getIPString(sender));
                            }
                        }
                        else // normal packet activity
                        {
                            /// \TODO: FLESH THIS OUT
                            switch(p->op_code)
                            {
                                case OP_RetransmissionAck:
                                {
                                    /// \TODO: Destroy sequence within connection
                                   Logger::getInstance().Log(Logs::DEBUG, "NetworkClient::updateLoop()", "Received retransmissions ACK! seqID [{}], pktNum [{}]", p->seqIdent, p->pktNum);
                                    m_serverConnection->addPacket(&p);
                                    break;
                                }
                                case OP_RetransmissionImpossible:
                                {
                                    /// \TODO: Destroy sequence within connection
                                   Logger::getInstance().Log(Logs::DEBUG, "NetworkClient::updateLoop()", "Received retransmissions impossible! seqID [{}], pktNum [{}]", p->seqIdent, p->pktNum);
                                    m_serverConnection->addPacket(&p);
                                    break;
                                }
                                case OP_ConnectionDisconnect:
                                {
                                    /// \TODO: Create way to handle server disconnecting this client
                                   Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkClient::updateLoop()", "Closing connection to server!");
                                    m_serverConnection->setClosed(true);
                                    break;
                                }
                                case OP_Ack: // sequence has completed or destroyed, let sender know
                                {
                                   Logger::getInstance().Log(Logs::DEBUG, "NetworkClient::updateLoop()", "Received ACK for seqID [{}].", p->seqIdent);
                                    m_serverConnection->addPacket(&p);
                                    break;
                                }
                                case OP_ConnectionAccepted: // clients may need multiple accepted replies
                                {
                                    if(sender != getDstSock())
                                    {
                                       Logger::getInstance().Log(Logs::DEBUG, "NetworkClient::updateLoop()", "Additional OP_ConnectionAccepted.");
                                        m_serverConnection->addPacket(&p);
                                    }
                                    else
                                    {
                                       Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkClient::updateLoop()", "Additional OP_ConnectionAccepted from base connection!");
                                    }
                                    break;
                                }
                                default: // normal packets
                                {
                                   Logger::getInstance().Log(Logs::DEBUG, "NetworkClient::updateLoop()", "'default' case hit, OP [{}]", p->op_code);
                                    m_serverConnection->addPacket(&p); // add packet to connection
                                    break;
                                }
                            }
                        }
                    }
                }

                // remove packet
                safeDelete(p);
                m_packetBuffer.pop();
                pkts--;
            }
        }

        // release lock
        updateLock.unlock();

       Logger::getInstance().Log(Logs::INFO, Logs::Network, "NetworkClient::updateLoop()", "Exiting updateLoop().");
    }

    /*void NetworkClient::listenLoop()
    {
        unsigned char* buffer = new unsigned char[PACKET_MAX_SIZE];
        struct sockaddr_storage sender;
        int bytes_read = 0;
        int retVal = 0;

        // poll()
        struct pollfd ufds[1]; // only listening port (for now...)
        ufds[0].fd = m_socket->getFD();
        ufds[0].events = POLLIN | POLLPRI;
        //ufds[0].revents = POLLERR | POLLHUP;

        while(m_isActive)
        {
            // if not accepting connections yet, keep skipping
            if(!m_netListening) { continue; }
            //std::unique_lock<std::mutex> loopLock(llmutex);

            // clear buffer and bytes_read, setup sender variable
            memset(buffer, 0, PACKET_MAX_SIZE);
            memset(&sender, 0, sizeof(struct sockaddr_storage));
            bytes_read = 0;
            retVal = 0;

            // actually poll for data
            retVal = poll(ufds, 1, m_pollTimeout); // parse all sockets, all 1 of them, never timeout

            if(retVal < 0) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkClient::listenLoop()", "poll() returned [{}], this is likely fatal. Stopping loop", retVal); m_isActive = false; }
            //else if(retVal == 0) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkClient::listenLoop()", "poll() timed out without data. This may be bad."); } /// \TODO: Is this important or even accurate?
            else if(retVal > 0 && ufds[0].revents & POLLIN)
            {
                bytes_read = m_socket->receive(&sender, buffer); // read in data
                if(bytes_read <= PACKET_MIN_RCV_SIZE) // catch spurious wake-ups or bad data
                {
                   Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkClient::listenLoop()", "bytes_read [{}] <= PACKET_MIN_RCV_SIZE [{}]", bytes_read, PACKET_MIN_RCV_SIZE);
                    continue;
                }

               Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "NetworkClient::listenLoop()", "--- NetworkClient::listenLoop: Bytes Read [{}] ---", bytes_read);
                Packet* p = new Packet(buffer, bytes_read, Time::getInstance().nowMS());

                // packet arrived from expected source and isn't broken
                if(isPacketValid(p) && isSameSource(&sender, m_dstAddress))
                {
                    // immediate reply of retry requests
                    if(p->op_code == OP_RetransmissionRequest)
                    {
                        bool found = false;
                        auto seq = m_storedSequences.find(p->seqIdent);

                        if(seq != m_storedSequences.end())
                        {
                            Packet* pkt = seq->second->getPacketByNumber(p->pktNum);
                            if(pkt)
                            {
                               Logger::getInstance().Log(Logs::DEBUG, "NetworkClient::listenLoop()", "Sending retransmission of seqID [{}], pkt# [{}]", pkt->seqIdent, pkt->pktNum);
                                sendRetryResponse(pkt);
                                found = true;
                            }
                        }

                        // packet sequence already destroyed
                        if(!found) { Logger::getInstance().Log(Logs::DEBUG, "NetworkClient::listenLoop()", "Sending retry impossible for seqID [{}]!", p->seqIdent); sendBuiltin(OP_RetransmissionImpossible, p->seqIdent, p->pktNum); }
                    }
                    else
                    {
                        PacketPair pp(&p, sender);
                        m_packetBuffer.push(pp);
                        m_updateCV.notify_one();
                    }
                }
                else
                {
                   Logger::getInstance().Log(Logs::DEBUG, "NetworkClient::listenLoop()", "Packet damaged or from unknown source [{}].", getIPString(&sender));
                }

                // cleanup, if not handed off
                safeDelete(p);
            }
        }

        safeDeleteArray(buffer);
       Logger::getInstance().Log(Logs::INFO, Logs::Network, "NetworkClient::updateLoop()", "Exiting listenLoop().");
        m_netListening = false;
        m_isActive = false;
        m_socket->closeSocket();
    }*/
}
