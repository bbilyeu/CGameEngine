#include "net/Network.h"
#include "srv/Security.h"

#include <iostream>

namespace CGameEngine
{
    /// Network ///////////////////////////////////////////////////////////////

    Network::Network(SoftwareVersion* swv, uint16_t port, SafeQueue<Datagram*>* dgbuff, std::string hostname /*= ""*/)
        : m_version(swv), m_srcPort(port), m_datagramBuffer(dgbuff)
    {
        if(!initSockets()) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "Network::Network()", "[Windows] Networking could NOT be started!"); }
        else
        {
            /// \NOTE: Passing port '0' is "use ephemeral ports"
            /// \TODO: Add support for multiple interfaces (bonding?)
            /// \TODO: Add config option to specify interface(s)
            if(port != 0) { if(!isPortOpen(m_srcPort)) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "Network::Network()", "Port {} is in use!", m_srcPort); } }

            // generate address
            generateAddress(hostname, m_srcPort, &m_srcAddress);
			Logger::getInstance().Log(Logs::DEBUG, "Network::Network()", "IP: {}", getIPString((struct sockaddr_storage*)m_srcAddress->ai_addr));

            // open socket
            m_socket = new NetSocket(m_srcAddress);
            if(!m_socket->isOpen()) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "Network::Network()", "Port {} could not be opened!", m_srcPort); }
            m_socketPairs[m_socket->getFD()] = std::make_pair(m_socket, m_datagramBuffer);

            // generate unique ID
            char* host = new char[128];
            gethostname(host, sizeof(host));
            std::string tmp = std::string(host);
            tmp += std::to_string(Time::getInstance().now());
            m_uniqueID = CRC32::create(tmp.c_str(), tmp.length());
            safeDeleteArray(host);

            // set listening active
            m_isActive = true;
        }
    }

    Network::~Network()
    {
        m_isActive = false;
        m_netListening = false;
        m_isTCP = false;
        for(SafeUnorderedMap<uint32_t, StoredSequence*>::iterator it = m_storedSequences.begin(); it != m_storedSequences.end(); ++it)
        {
            safeDelete(it->second);
        }
        m_storedSequences.clear();

        if(m_listenThread)
        {
            while(!m_listenThread->joinable()) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkServer::shutdownSockets()", "Listen (Net) Thread is not joinable."); } // waiting...
            m_listenThread->join();
            safeDelete(m_listenThread);
        }
        if(m_updateThread)
        {
            while(!m_updateThread->joinable()) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkServer::shutdownSockets()", "Update Thread is not joinable."); } // waiting...
            m_updateThread->join();
            safeDelete(m_updateThread);
        }
        if(m_sendThread)
        {
            while(!m_sendThread->joinable()) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkServer::shutdownSockets()", "Send Thread is not joinable."); } // waiting...
            m_sendThread->join();
            safeDelete(m_sendThread);
        }

        #if PLATFORM == PLATFORM_WINDOWS
                WSACleanup();
        #endif

        /*if(m_listenThread) { m_listenThread->join(); safeDelete(m_listenThread); }
        if(m_sendThread) { m_sendThread->join(); safeDelete(m_sendThread); }
        if(m_updateThread) { m_updateThread->join(); safeDelete(m_updateThread); }*/
        if(m_srcAddress) { freeaddrinfo(m_srcAddress); m_srcAddress = nullptr; }
        if(m_bcAddress) { freeaddrinfo(m_bcAddress); m_bcAddress = nullptr; }
        safeDelete(m_socket);
    }

    void Network::stop()
    {
        m_isActive = false;
        m_netListening = false;
        m_pollTimeout = 0;
        m_sendCV.notify_one();
        m_updateCV.notify_one();
    }

    void Network::generateAddress(const std::string addr, uint16_t port, addrinfo** dst)
    {
        struct addrinfo hints;
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        if(m_isTCP)
        {
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
        }
        else // UDP
        {
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_protocol = IPPROTO_UDP;
        }

        // ai_flags
        if(addr == "") { hints.ai_flags = AI_PASSIVE; }
        else { hints.ai_flags = AI_CANONNAME; }

        int ret = 0;
        if(addr != "" && port != 0)
        {
            ret = getaddrinfo(addr.c_str(), std::to_string(port).c_str(), &hints, dst);
           Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::generateAddress()", "Called with addr [{}], port [{}]", addr, port);
        }
        else if(addr != "" && port == 0)
        {
            ret = getaddrinfo(addr.c_str(), nullptr, &hints, dst);
           Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::generateAddress()", "Called with addr [{}], but without a port.", addr);
        }
        else if(addr == "" && port > 0)
        {
            ret = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, dst);
           Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::generateAddress()", "Called without a hostname/IP, but with port [{}].", port);
        }
        else
        {
            //ret = getaddrinfo(nullptr, nullptr, &hints, dst);
           Logger::getInstance().Log(Logs::FATAL, Logs::Network, "Network::generateAddress()", "Called without a hostname or port!");
        }

        // exit on failure
        if(ret != 0) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "Network::generateAddress()", "Returned [{}]. ERROR: {}.", ret, gai_strerror(ret)); }
    }

    // differs by forcing IPv4, UDP.
    // in absence of hostname, falls back to first available 10.x address
    void Network::generateInternalAddress(uint16_t port, addrinfo** dst, std::string hostname /*= ""*/)
    {
		Logger::getInstance().Log(Logs::DEBUG, "Network::generateInternalAddress()", "Port [{}], Hostname [{}]", port, hostname);
        struct addrinfo hints;
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_INET; // ipv4
        if(m_isTCP)
        {
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
        }
        else // UDP
        {
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_protocol = IPPROTO_UDP;
        }

        // ai_flags
        if(hostname == "") { hints.ai_flags = AI_PASSIVE; }
        else { hints.ai_flags = AI_CANONNAME; }

        int ret = 0;
        if(hostname != "" && port != 0)
        {
            ret = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, dst);
           Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::generateInternalAddress()", "Called hostname addr [{}], port [{}]", hostname, port);
        }
        else if(hostname == "" & port > 0)
        {
            ret = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, dst);
           Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::generateInternalAddress()", "Called with port [{}], but no hostname", port);
        }
        else
        {
            //ret = getaddrinfo(nullptr, nullptr, &hints, dst);
           Logger::getInstance().Log(Logs::FATAL, Logs::Network, "Network::generateInternalAddress()", "Called without a hostname or port!");
        }

        // exit on failure
        if(ret != 0) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "Network::generateInternalAddress()", "Returned [{}]. ERROR: {}.", ret, gai_strerror(ret)); }
    }

    bool Network::isPortOpen(uint16_t port)
    {
        bool retVal = false;
        struct addrinfo* ai;
        generateAddress("", port, &ai);

        if(ai)
        {
            int handle = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            if(handle >= 0 && bind(handle, ai->ai_addr, ai->ai_addrlen) == 0)
            {
                // flush errors, pre closing
                int err = 1; socklen_t len = sizeof(err);
                getsockopt(handle, SOL_SOCKET, SO_ERROR, (char*)&err, &len);
                #if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_LINUX
                    shutdown(handle, SHUT_RDWR);
                    close(handle);
                #elif PLATFORM == PLATFORM_WINDOWS
                    closesocket(handle);
                #endif

                // update return value
				Logger::getInstance().Log(Logs::DEBUG, "Network::isPortOpen()", "Port [{}] is open!", port);
                retVal = true;
            }
        }

        // cleanup
        if(ai) { freeaddrinfo(ai); ai = nullptr; }

        if(!retVal) { Logger::getInstance().Log(Logs::DEBUG, "Network::isPortOpen()", "Port [{}] is closed!", port); }
        return retVal;
    }

    void Network::listenLoop()
    {
        unsigned char* buffer = new unsigned char[PACKET_MAX_SIZE];
        struct sockaddr_storage sender;
        int bytes_read = 0;
        int retVal = 0;

        // poll()
        struct pollfd ufds[1];
        ufds[0].fd = m_socket->getFD();
        ufds[0].events = POLLIN;

        while(m_isActive)
        {
            // if not accepting connections yet, keep skipping
            if(!m_netListening) { continue; }

            // clear buffer and bytes_read, setup sender variable
            memset(buffer, 0, PACKET_MAX_SIZE);
            memset(&sender, 0, sizeof(struct sockaddr_storage));
            bytes_read = 0;
            retVal = 0;

            // actually poll for data
            retVal = poll(ufds, 1, m_pollTimeout); // parse all sockets, all 1 of them, never timeout

            if(retVal < 0) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "Network::listenLoop()", "poll() returned [{}], this is likely fatal. Stopping loop", retVal); m_isActive = false; }
            //else if(retVal == 0) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::listenLoop()", "poll() timed out without data. This may be bad."); } /// \TODO: Is this important or even accurate?
            else if(retVal > 0 && ufds[0].revents & POLLIN)
            {
                bytes_read = m_socket->receive(&sender, buffer); // read in data
                if(bytes_read <= PACKET_MIN_RCV_SIZE) { continue; } // catch too small data and abandon

                //Logger::getInstance().Log(Logs::DEBUG, "Network::listenLoop()", "--- Network::listenLoop: Bytes Read [{}] ---", bytes_read);
                Packet* p = new Packet(buffer, bytes_read, Time::getInstance().nowMS());

                if(isPacketValid(p, &sender))
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
								Logger::getInstance().Log(Logs::DEBUG, "Network::listenLoop()", "Sending retransmission of seqID [{}], pkt# [{}]", pkt->seqIdent, pkt->pktNum);
                                sendRetryResponse(&sender, pkt);
                                found = true;
                            }
                        }
                        else { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::listenLoop()", "Retransmission request from invalid source! seqID [{}], pkt# [{}]", p->seqIdent, p->pktNum); }

                        // packet sequence already destroyed
                        if(!found) { Logger::getInstance().Log(Logs::DEBUG, "Network::listenLoop()", "Sending retry impossible for seqID [{}]!", p->seqIdent); sendBuiltin(&sender, OP_RetransmissionImpossible, p->seqIdent, p->pktNum); }
                    }
                    else // store packet for processing within updateLoop
                    {
                        //Logger::getInstance().Log(Logs::DEBUG, "Network::listenLoop()", "\033[97mReceived Packet with OP Code '{}'\033[0m", p->op_code);
                        PacketPair pp(&p, sender);
                        m_packetBuffer.push(pp);
                        m_updateCV.notify_one();
                    }
                }
                else
                {
                   Logger::getInstance().Log(Logs::DEBUG, "Network::listenLoop()", "Packet was damaged or had incorrect SoftwareVersion ({})", !p->matchesVersion(m_version));
                }

                // cleanup, if not handed off
                safeDelete(p);
            }
        }

        safeDeleteArray(buffer);
       Logger::getInstance().Log(Logs::INFO, Logs::Network, "Network::listenLoop()", "Exiting listenLoop().");
        m_netListening = false;
        m_isActive = false;
        m_socket->closeSocket();
    }

    void Network::sendLoop()
    {
        struct pollfd ufds[1];
        ufds[0].fd = m_socket->getFD();
        ufds[0].events = POLLOUT;
        int retVal = 0;

        std::mutex slmutex;
        std::unique_lock<std::mutex> sendLock(slmutex);
    	while (m_isActive)
        {
            if(m_sendQueue.empty()) { m_sendCV.wait(sendLock); }

            while(!m_sendQueue.empty())
            {
                retVal = poll(ufds, 1, m_pollTimeout);
                if(retVal < 0) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkClient::sendLoop()", "poll() returned [{}], this is likely fatal. Stopping loop", retVal); m_isActive = false; }
                else if(retVal > 0 && ufds[0].revents & POLLOUT)
                {
                    //Logger::getInstance().Log(Logs::DEBUG, "NetworkClient::sendLoop()", "Packet Data:\n", dumpPacket(m_sendQueue.front().data, m_sendQueue.front().pSize));
                    m_socket->sendData(m_sendQueue.front().addr, m_sendQueue.front().data, m_sendQueue.front().pSize);
                    m_sendQueue.pop();
                }
            }
        }
        sendLock.unlock();
    }

    void Network::broadcast(uint16_t port, uint16_t opCode, unsigned char** data, uint32_t dataLength)
    {
        if(!m_socket || !data || dataLength == 0) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::broadcast()", "No valid NetSocket."); return; }
        else if(!data || (*data) == nullptr) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::broadcast()", "No data passed."); return; }
        else if(dataLength == 0 || dataLength > PACKET_DATA_SIZE) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::broadcast()", "Invalid data length! [{}]", dataLength); return; }

        // take ownership
        unsigned char* d = *data;
        *data = nullptr;

        // get CRC of data
        uint32_t totalCRC = CRC32::create(d, dataLength);

        // create packet and dump to buffer
        Packet pkt(m_identifier, m_version, opCode, Time::getInstance().nowMS(), getSequenceID(), 1, 1, totalCRC, dataLength, dataLength, d);
        pkt.serializeOut();

        // create address (if needed)
        if(!m_bcAddress)
        {
            std::string ip = getIPString((struct sockaddr_storage*)m_srcAddress->ai_addr);
            generateAddress(ip, port, &m_bcAddress);
        }

        // send it, broadcast style
        bool results = m_socket->broadcastSend((sockaddr_storage*)m_bcAddress->ai_addr, pkt.buffer, pkt.pSize);
        if(!results) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "Network::broadcast()", "Failed to send broadcast data!"); }

        // trash data
        safeDeleteArray(d);
    }

    /*void Network::send(sockaddr_storage* addr, uint16_t opCode, void* data)
    {
        if(!addr) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::send(void*)", "No address! data [{}]", data); return; }
        else if(!data) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "Network::send(void*)", "data is nullptr! Ignoring send() call."); return; }

        std::stringstream ss;

        // serialize via cereal
        {
            cereal::PortableBinaryOutputArchive oarc(ss);
            data.serialize(oarc);
        }

        // convert to unsigned char array
        uint32_t len = ss.str().length();
        unsigned char* data = new char[len];
        memcpy(data, ss.str().c_str(), len);

        // send then cleanup
        Network::send(addr, opCode, data, len);
        safeDeleteArray(len);
    }*/

    /// \TODO: Review this for efficiency improvements
    void Network::send(sockaddr_storage* addr, uint16_t opCode, unsigned char** data, uint32_t dataLength)
    {
        if(!addr && !m_isTCP) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::send()", "No address. Ignoring send() call."); return; }
        //else if(!m_SERVER && !m_isConnectionAccepted) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::send()", "Connection not yet accepted by server, ignoring send() call!"); }
        else if(!data || *data == nullptr) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "Network::send()", "data is nullptr! Ignoring send() call."); return; }
        else if(dataLength == 0) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::send()", "Passed length is 0! Ignoring send() call."); return; }
        else if(m_isTCP && !m_isConnected) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::send()", "TCP connection not accepted! Ignoring send() call."); return; }

        // take ownership
        unsigned char* d = *data;
        *data = nullptr;

        // vector for packets, if necessary
        std::queue<Packet*> packets;

        // get sequence ID
        uint32_t seq_ID = getSequenceID();

        // blanks/misc
        int payloads = 1;
        uint16_t leftoverSlice = 0;
        int position = 0; // position in passed data buffer

        // get number of packets
        if(dataLength+PACKET_HEADER_SIZE > PACKET_MAX_SIZE)
        {
            payloads = static_cast<int>(dataLength / PACKET_DATA_SIZE);
            leftoverSlice = dataLength - (PACKET_DATA_SIZE * payloads); // invert the remaining the value
            payloads++; // to account for remainder
        }

        // define slice size per payload
        int sliceLength = (payloads == 1) ? dataLength : PACKET_DATA_SIZE;
        unsigned char* slice = new unsigned char[sliceLength];

        // generate CRC value for whole of data
        uint32_t totalCRC = CRC32::create(d, dataLength);
        uint64_t timestamp = Time::getInstance().nowMS();

        //Logger::getInstance().Log(Logs::DEBUG, "Network::send()", "payloads [{}], sliceLength [{}], leftoverSlice [{}], dataLength [{}], m_identifier [{}], seq_ID [{}], crc [{}]", payloads, sliceLength, leftoverSlice, dataLength, m_identifier, seq_ID, totalCRC);

        for(int p = 0; p < payloads; p++)
        {
            // account for last slice and potential variable length
            if(payloads > 1 && p+1 == payloads)
            {
                sliceLength = leftoverSlice;
                safeDeleteArray(slice);
                slice = new unsigned char[sliceLength];
            }

            // reset our "slice"
            memset(slice, 0, sliceLength);

            // load the slice
            if(position+sliceLength > dataLength) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::send()", "Misaligned slice length! sliceLength [{}], dataLength [{}], position [{}]", sliceLength, dataLength, position); }
            memcpy(slice, d+position, sliceLength);

            // create crc
            uint32_t crc = CRC32::create(slice, sliceLength);

            // create a packet
            Packet* pkt = new Packet(m_identifier, m_version, opCode, timestamp, seq_ID, p, payloads, totalCRC, sliceLength, dataLength, slice);

            // denote next position in the data buffer
            position += sliceLength;

            // store it
            packets.push(pkt);
        }

        /// \TODO: Build system to push the entire vector back for recalling with 5s lifetime
        if(opCode != OP_RetransmissionReply && payloads > 1)
        {
            StoredSequence* ss = new StoredSequence(seq_ID, payloads, timestamp, packets);
            if(ss) { m_storedSequences.insert(std::make_pair(seq_ID, ss)); }
            ss = nullptr;
        }

        // generate unsigned char* per packet to send
        int pSize = sliceLength+PACKET_HEADER_SIZE;
        while(packets.size() > 0)
        {
            Packet* p = packets.front();
            // account for final "slice" of variable length
            if(leftoverSlice != 0 && packets.size() == 1)
            {
                pSize = leftoverSlice+PACKET_HEADER_SIZE;
                if(pSize > PACKET_MAX_SIZE) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::send()", "pSize for leftover packet is LARGER than max_length allowed!"); }
            }

            // dump data to uchar* arrays for transmission
            p->serializeOut();
            //m_socket->sendData(addr, p->buffer, p->pSize);
            m_sendQueue.emplace(addr, p->buffer, p->pSize);

            // next packet
            packets.front() = nullptr;
            packets.pop();
        }
        m_sendCV.notify_one(); // tell sendQueue to process packets
        safeDeleteArray(slice);
        safeDeleteArray(d);
    }

    void Network::sendBuiltin(sockaddr_storage* addr, uint16_t opCode, uint32_t seqID /*= 0*/, uint32_t pktNum /*= 0*/)
    {
        if(!addr && !m_isTCP) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::sendBuiltin()", "No address passed. Ignoring send() call."); return; }
        else if(m_isTCP && !m_isConnected) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::sendBuiltin()", "TCP connection not accepted! Ignoring send() call."); return; }
        else if(opCode >= OP_ENDINTERNAL) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::sendBuiltin()", "Passed OPCode was not a builtin! Max is {}, passed was {}. sendSimple() should be used instead.", OP_ENDINTERNAL, opCode); return; }

        // temp variable
        unsigned char* data = nullptr;
        uint16_t dataLength = 0;
        uint32_t pktTotal = 1;

        // switch for logging or special sends
        switch(opCode)
        {
            case OP_RetransmissionRequest:
            {
                //Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::sendBuiltin(no data)", "[Dst: {}] Sending retry request for seq [{}], pktNum [{}]", getIPString(addr), seqID, pktNum);
                break;
            }
            case OP_RetransmissionImpossible:
            {
                //Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::sendBuiltin(no data)", "[Dst: {}] Sending retry impossible for seq [{}], pktNum [{}]", getIPString(addr), seqID, pktNum);
                break;
            }
            case OP_RetransmissionAck:
            {
                //Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::sendBuiltin(no data)", "[Dst: {}] Sending retry ACK for seq [{}], pktNum [{}]", getIPString(addr), seqID, pktNum);
                break;
            }
            case OP_Ack:
            {
                //Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::sendBuiltin(no data)", "[Dst: {}] Sending Ack for seq [{}]", getIPString(addr), seqID);
                break;
            }
            case OP_KeepAlive:
            {
                //Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::sendBuiltin(no data)", "[Dst: {}] Sending KeepAlive", getIPString(addr));
                break;
            }
            case OP_ConnectionAccepted:
            {
                //Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::sendBuiltin(no data)", "[Dst: {}] Sending ConnectionAccepted", getIPString(addr));
                break;
            }
            case OP_ConnectionDisconnect:
            {
                //Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::sendBuiltin(no data)", "[Dst: {}] Sending ConnectionDisconnect", getIPString(addr));
                break;
            }
            case OP_ConnectionRequest:
            {
                // create struct
                ConnectionRequest_Struct cr;
                dataLength = sizeof(ConnectionRequest_Struct);
                memset(&cr, 0, dataLength);

                // set title
                memcpy(cr.title, m_title.c_str(), m_title.length());

                // get version
                cr.version = (*m_version);

                // get or create unique identifier
                if(m_uniqueID != 0) { cr.uniqID = m_uniqueID; }
                else
                {
                    /// \TODO: ENSURE THIS NEVER HAPPENS
                    char* host = new char[128];
                    gethostname(host, sizeof(host));
                    std::string tmp = std::string(host);
                    tmp += std::to_string(Time::getInstance().now());
                    m_uniqueID = CRC32::create(tmp.c_str(), tmp.length());
                    cr.uniqID = m_uniqueID;
                   Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::sendBuiltin(no data)", "Created uniqID [{}]", m_uniqueID);
                    safeDelete(host);
                }

                // create payload
                data = new unsigned char[dataLength];
                memset(data, 0, dataLength);
                memcpy(data, &cr, dataLength);

                //Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::sendBuiltin(no data)", "[Dst: {}] Sending ConnectionRequest, title [{}], uniqID [{}]", getIPString(addr), cr.title, cr.uniqID);
                break;
            }
        }

        // if no data, falsify some
        if(!data || dataLength == 0) { dataLength = sizeof(m_uniqueID); data = new unsigned char[dataLength]; memcpy(data, &m_uniqueID, dataLength); } // junk data

        // generate CRC
        uint32_t totalCRC = CRC32::create(data, dataLength);

        // create packet
        Packet pkt(m_identifier, m_version, opCode, Time::getInstance().nowMS(), seqID, pktNum, pktTotal, totalCRC, dataLength, dataLength, data);
        pkt.serializeOut();

        // send and cleanup
        m_socket->sendData(addr, pkt.buffer, pkt.pSize);
        safeDeleteArray(data);
    }

    void Network::sendRetryResponse(sockaddr_storage* addr, Packet* p)
    {
         if(!addr && !m_isTCP) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::sendRetryResponse()", "No address!"); return; }
         else if(!p) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::sendRetryResponse()", "No packet passed!"); return; }
         else if(m_isTCP && !m_isConnected) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::send()", "TCP connection not accepted! Ignoring send() call."); return; }

        // create actual payload
        p->serializeOut();

       Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Network::sendRetryResponse()", "Sending retry response for seq [{}], pktNum [{}]", p->seqIdent, p->pktNum);
        //m_socket->sendData(addr, p->buffer, p->pSize);
        m_sendQueue.emplace(addr, p->buffer, p->pSize);
        m_sendCV.notify_one(); // tell sendQueue to process packets
    }

    void Network::sendSimple(sockaddr_storage* addr,  uint16_t opCode)
    {
        if(!addr && !m_isTCP) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::sendSimple(no data)", "No address. Ignoring send() call."); return; }
        else if(m_isTCP && !m_isConnected) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Network::sendSimple()", "TCP connection not accepted! Ignoring send() call."); return; }

        // junk data
        uint16_t dataLength = sizeof(m_uniqueID);
        unsigned char* data = new unsigned char[dataLength];
        memcpy(data, &m_uniqueID, dataLength);
        uint32_t pktTotal = 1;

        // generate CRC
        uint32_t totalCRC = CRC32::create(data, dataLength);

        // create packet
        Packet pkt(m_identifier, m_version, opCode, Time::getInstance().nowMS(), 0, 0, pktTotal, totalCRC, dataLength, dataLength, data);
        pkt.serializeOut();

        // send and cleanup
        m_socket->sendData(addr, pkt.buffer, pkt.pSize);
        safeDeleteArray(data);
    }

    void Network::setAccepting(bool val /*= true*/)
    {
        m_netListening = val;
        m_updateCV.notify_one();
        m_sendCV.notify_one();
    }

    void Network::setTitle(std::string str)
    {
        m_title = str;
        std::string hashVal = strSHA256(str);
        m_identifier = hashVal.substr(0, 4);
    }

    /// Network private functions below ///////////////////////////////////////

    bool Network::initSockets()
    {
        #if PLATFORM == PLATFORM_WINDOWS
            WSADATA WsaData;
            return WSAStartup( MAKEWORD(2,2), &WsaData ) == NO_ERROR;
        #else
            return true;
        #endif
    }

    /// \TODO: Should this be mutex'ed to prevent multiple seqIDs being reused at once?
    uint32_t& Network::getSequenceID()
    {
        if(m_seqID >= MAX_SEQ_ID) { m_seqID = MIN_SEQ_ID; } else { m_seqID++; }
        return m_seqID;
    }
}
