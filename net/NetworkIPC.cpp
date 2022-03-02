#include "NetworkIPC.h"

#include "common/Timer.h"

namespace CGameEngine
{
    /// create primary (left) side
    NetworkIPC::NetworkIPC(std::string sockname, Network* net, SafeQueue<UnixPacket*>* packetQueue)
        : m_isPrimary(true), m_sockName(sockname), m_net(net), m_unixPackets(packetQueue)
    {
        std::string tmpSock = m_sockName + "A";
        m_readSocket = new UnixSocket(tmpSock);
        if(m_readSocket->isOpen())
        {
            m_isActive = true;
            generateID();
           Logger::getInstance().Log(Logs::INFO, Logs::Network, "NetworkIPC::NetworkIPC(str)", "Unix Socket opened [id {}, fd {}]", m_uniqueID, m_readSocket->getFD());
            m_thread = new std::thread(startLoop, this);
        }
        else { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkIPC::NetworkIPC(string, Network, SafeQueue)", "readSocket failed to open!"); }
    }

    /// create secondary (right) side
    NetworkIPC::NetworkIPC(std::string sockname, SafeQueue<UnixPacket*>* packetQueue)
        : m_sockName(sockname), m_unixPackets(packetQueue)
    {
        // connect to the write socket from primary (left) side
        std::string tmpSock = m_sockName + "A";
        m_writeSocket = new UnixSocket(tmpSock, false);
        if(m_writeSocket->isOpen())
        {
            m_isActive = true;
            sendSimple(OP_KeepAlive);
        }
        else { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkIPC::NetworkIPC(string, SafeQueue)", "writeSocket failed to open!"); }
    }

    NetworkIPC::~NetworkIPC()
    {
        while(!shutdownSockets())
        {
           Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkIPC::~NetworkIPC()", "Waiting on shutdownSockets() to complete.");
        }

        if(m_thread)
        {
            while(!m_thread->joinable()) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkIPC::shutdownSockets()", "mainLoop thread is not joinable."); } // waiting...
            m_thread->join();
            safeDelete(m_thread);
        }

        if(m_readSocket)
        {
            m_readSocket->closeSocket();
            safeDelete(m_readSocket);
        }

        if(m_writeSocket)
        {
            m_writeSocket->closeSocket();
            safeDelete(m_writeSocket);
        }

        m_net = nullptr;
    }

    bool NetworkIPC::shutdownSockets()
    {
        m_isActive = false;
        m_isConnected = false;
        while(!m_unixPackets->empty()) { safeDelete(m_unixPackets->front()); m_unixPackets->pop(); }
       Logger::getInstance().Log(Logs::DEBUG, "NetworkIPC::shutdownSockets()", "Completed Successfully!");
        return true;
    }

    void NetworkIPC::mainLoop()
    {
        // poll variables
        struct pollfd ufds[1];
        ufds[0].fd = m_readSocket->getFD();
        ufds[0].events = POLLIN;
        int retVal = 0;
        bool destroyPacket = true;

        // data variables
        unsigned char* buffer = new unsigned char[UNIX_PACKET_MAX_SIZE];
        int bytes_read = 0;

        while(m_isActive)
        {
            // clear buffer and bytes_read variable
            memset(buffer, 0, PACKET_MAX_SIZE);
            bytes_read = 0;
            retVal = 0;
            destroyPacket = true;

            // actually poll for data
            retVal = poll(ufds, 1, m_pollTimeout);

            if(retVal < 0) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkIPC::mainLoop()", "poll() returned [{}], this is likely fatal. Stopping loop", retVal); m_isActive = false; }
            else if(retVal > 0 && ufds[0].revents & POLLIN) // normal data
            {
                bytes_read = m_readSocket->receive(buffer);
                if(bytes_read <= 0) { continue; } // too small or empty

                // build packet and process it
                UnixPacket* up = new UnixPacket(buffer, bytes_read);
                if(!m_isConnected && up->op_code == OP_KeepAlive)
                {
                   Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "NetworkIPC::mainLoop()", "Initial OP_KeepAlive, setting connected to true.");
                    m_isConnected = true;
                    m_unixPackets->push(up);
                    destroyPacket = false;
                }
                else if(m_isConnected)
                {
                    switch(up->op_code)
                    {
                        case OP_KeepAlive:
                        {
                            // primary (left) sends it in, secondary (right) replies
                            if(!m_isPrimary) { sendSimple(OP_KeepAlive, 0); }
                            else { m_unixPackets->push(up); destroyPacket = false; }
                            break;
                        }
                        case OP_IPCData:
                        {
                            if(m_uniqueID == 0) { m_uniqueID = up->senderID; }
                            else { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkIPC::mainLoop()", "OP_IPCData received, but unique ID ({}) already set!", m_uniqueID); }
                            break;
                        }
                        default:
                        {
                            m_unixPackets->push(up);
                            if(m_cv) { m_cv->notify_one(); }
                            destroyPacket = false;
                            break;
                        }
                    }
                }

                // cleanup
                if(destroyPacket) { safeDelete(up); }
            }
            else if(retVal > 0)
            {
               Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkIPC::mainLoop()", "poll() returned [{}], with revents [{}]. Is this expected?", retVal, ufds[0].revents);
            }
        }

        // cleanup
        safeDeleteArray(buffer);
       Logger::getInstance().Log(Logs::INFO, Logs::Network, "NetworkIPC::mainLoop()", "Exiting mainLoop().");
        m_readSocket->closeSocket();
    }

    /// Network (Process A) -> Process B
    void NetworkIPC::addDatagram(Datagram** dg)
    {
        if(!m_isActive) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkIPC::addDatagram()", "isActive is false."); return; } // closing
        else if(!dg || !(*dg)) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkIPC::addDatagram()", "Passed Datagram pointer or pointer to pointer was null!"); return; }

        // take ownership
        Datagram* d = *dg;
        *dg = nullptr;

        // create UnixPacket and send
        UnixPacket* up = new UnixPacket(d->op_code, d->timestamp, d->senderUniqID, d->dataLength, d->data);
        up->serializeOut();
        m_readSocket->sendData(up->buffer, up->pSize);

        // cleanup
        safeDelete(d);
        safeDelete(up);
    }

    /// create missing socket (read or write)
    void NetworkIPC::completePair()
    {
        // if shutting down, just let it die
        if(!m_isActive) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkIPC::completePair()", "isActive is false."); return; }

        // if no write socket on primary (left), open one
        if(m_isPrimary)
        {
            if(!m_writeSocket)
            {
                // this is run on the primary (left) side to
                // connect to the secondary (right) side
                std::string tmpSock = m_sockName + "B";
                m_writeSocket = new UnixSocket(tmpSock, false);
                if(m_writeSocket->isOpen())
                {
                    m_isConnected = true;
                    sendSimple(OP_KeepAlive);
                    sendSimple(OP_IPCData, m_uniqueID);
                   Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "NetworkIPC::completePair()", "Primary (left) side completed as writeSocket connected.");
                }
                else
                {
                   Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkIPC::completePair()", "writeSocket failed to produce a file descriptor!");
                    return;
                }

            }
            else // has writeSocket
            {
                if(m_writeSocket->isOpen())
                {
                    m_isConnected = true;
                    sendSimple(OP_KeepAlive);
                    sendSimple(OP_IPCData, m_uniqueID);
                   Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "NetworkIPC::completePair()", "Primary (left) side re-established writeSocket connection.");
                }
                else
                {
                   Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkIPC::completePair()", "writeSocket has failed silently (was up, now down)!");
                    return;
                }
            }
        }
        // if no read socket on secondary (right), open one
        else if(!m_isPrimary)
        {
            if(!m_readSocket)
            {
                // this is run on the secondary (right) side to
                // create a socket for the primary (left) to connect
                std::string tmpSock = m_sockName + "B";
                m_readSocket = new UnixSocket(tmpSock);
                if(m_readSocket->isConnected())
                {
                    m_isConnected = true;
                    sendSimple(OP_KeepAlive);
                   Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "NetworkIPC::completePair()", "Secondary (right) side completed as readSocket connected.");
                    m_uniqueID = 0; // resetting as the primary (left) side will provide this
                    m_thread = new std::thread(startLoop, this);
                }
                else
                {
                   Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkIPC::completePair()", "readSocket failed to accept an incoming connection!");
                    return;
                }
            }
            else
            {
               Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkIPC::completePair()", "Read ({}) and Write ({}) socket exist, cannot process completePair().", m_readSocket->getFD(), m_writeSocket->getFD());
                return;
            }
        }
    }

    /// basic OPCode responses
    void NetworkIPC::sendSimple(uint16_t opCode, uint32_t sender /*= 0*/)
    {
        if(!m_isActive || !m_writeSocket)
        {
           Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkIPC::sendSimple()", "isActive is [{}] and/or no writeSocket ({}). OP_Code [{}]", m_isActive, (void*)m_writeSocket, opCode);
            return;
        }

        // create UnixPacket, send, and then cleanup
        UnixPacket* up = new UnixPacket(opCode, 0, sender, 0, nullptr);
        up->serializeOut();
        m_writeSocket->sendData(up->buffer, up->pSize);
        safeDelete(up);
    }

    /// Process B -> Network (Process A)
    void NetworkIPC::send(uint16_t opCode, uint32_t senderID, unsigned char** data, uint16_t dataLength, uint64_t arrival /*= 0*/)
    {
        if(!m_isActive || !m_writeSocket)
        {
           Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkIPC::send()", "isActive is [{}] and/or no writeSocket ({}).", m_isActive, (void*)m_writeSocket);
            return;
        }
        else if(!data || !(*data)) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkIPC::send()", "Passed data pointer was null!"); return; }
        else if(dataLength > UNIX_PACKET_DATA_SIZE) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkIPC::send()", "Passed data is too large! Received {} bytes, max is {}.", dataLength, UNIX_PACKET_DATA_SIZE); return; }

        // take ownership
        unsigned char* d = *data;
        *data = nullptr;

        UnixPacket up(opCode, arrival, senderID, dataLength, d);
        up.serializeOut();
        m_writeSocket->sendData(up.buffer, up.pSize);
        safeDelete(d);
    }

    bool NetworkIPC::isReadConnected()
    {
        if(m_isActive && m_readSocket && m_readSocket->getFD() != -1) { return true; }
        else { return false; }
    }

    bool NetworkIPC::isWriteConnected()
    {
        if(m_isActive && m_writeSocket && m_writeSocket->getFD() != -1) { return true; }
        else { return false; }
    }


    /// NetworkIPC private functions //////////////////////////////////////////

    void NetworkIPC::generateID()
    {
        // if exists or if right side (secondary), return
        if(m_uniqueID > 0) { return; }

        // generate unique ID
        char* host = new char[128];
        gethostname(host, sizeof(host));
        std::string tmp = std::string(host);
        tmp += m_sockName;
        m_uniqueID = CRC32::create(tmp.c_str(), tmp.length());
        safeDeleteArray(host);
    }
}
