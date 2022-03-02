#include "net/Connection.h"
#include <algorithm> // find, ceil, floor

#include "srv/Time.h"
#include "common/CRC32.h"

namespace CGameEngine
{
    /// Connection ////////////////////////////////////////////////////////////

    bool Connection::update(uint64_t timestamp)
    {
        // if retry timeout is 10+ seconds, the connection should be assumed lost
        // same applies to last received packet
        if(m_isClosing)
        {
           Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Connection::update()", "Received disconnect packet, m_isClosing is [{}].", m_isClosing);
            simpleDatagram(OP_ConnectionDisconnect);
            return true;
        }
        else if(m_retryTimeout > 10000)
        {
           Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Connection::update()", "Lagging too hard and timed out, m_retryTimeout is [{}].", m_retryTimeout);
            simpleDatagram(OP_ConnectionTimedOut);
            return true;
        }
        else if(timestamp > m_lastArrival && (timestamp - m_lastArrival) > 15000)
        {
           Logger::getInstance().Log(Logs::INFO, Logs::Network, "Connection::update()", "Connection lost, lastArrival was [{}] seconds ago (limit is 15s).", (float)(timestamp - m_lastArrival));
            simpleDatagram(OP_ConnectionLost);
            return true;
        }
        else if(!m_buffer)
        {
           Logger::getInstance().Log(Logs::CRIT, Logs::Network, "Connection::update()", "No Datagram buffer set!", m_isClosing);
            simpleDatagram(OP_ConnectionDisconnect);
            return true;
        }

        // snapshotting packet queue size to prevent out of range errors
        int queueSize = m_packets.size();

        // iterate over packets in "my" queue
        for(unsigned int i = 0; i < queueSize; i++)
        {
            Packet* p = m_packets.front();
            bool destroy = true;

            // is packet damaged? If so, request new one
            if(p->isDamaged())
            {
                RetryRequest_Struct* rr = new RetryRequest_Struct(p->seqIdent, p->pktNum, timestamp);
                m_retryRequests.insert( std::make_pair(p->seqIdent, rr) );
                rr = nullptr; // let Connection "own" it
            }
            // check for existing sequence or existing expired sequence
            else if(!doesExist(p->seqIdent))
            {
                // build a sequence
               Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Connection::update()", "Creating PacketSequence for seqID [{}], with [{}] packets", p->seqIdent, p->pktTotal);
                m_sequences.insert(std::make_pair(p->seqIdent, PacketSequence(p->seqIdent, p->pktTotal, timestamp, this)));
                m_sequences.at(p->seqIdent).addPacket(&p, timestamp);
            }
            else // sequence does exist, handle normally
            {
                if(p->op_code == OP_RetransmissionImpossible)
                {
                   Logger::getInstance().Log(Logs::WARN, Logs::Network, "Connection::update()", "Retry Impossible received for seqID [{}], destroying.", p->seqIdent);
                    m_sequences.erase(p->seqIdent);
                    m_expiredSequences.insert(std::make_pair(p->seqIdent, timestamp+5000));
                }
                else { m_sequences.at(p->seqIdent).addPacket(&p, timestamp); }
            }

            // cleanup
            safeDelete(p);
            m_packets.pop();
        }

        // iterate over sequences to kill or request retransmission
        std::queue<uint32_t> toBeRemoved;
        for(std::map<uint32_t, PacketSequence>::iterator it = m_sequences.begin(); it != m_sequences.end(); ++it)
        {
            bool destroy = it->second.update(timestamp);
            if(destroy) { toBeRemoved.push(it->first); }
        }

        // remove expired/completed sequences
        while(toBeRemoved.size() > 0)
        {
           Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Connection::update()", "Destroying PacketSequence [{}]", toBeRemoved.front());
            m_sequences.erase(toBeRemoved.front());
            m_retryRequests.erase(toBeRemoved.front());
            m_expiredSequences.insert(std::make_pair(toBeRemoved.front(), timestamp+5000));
            toBeRemoved.pop();
        }

        /// \NOTE: toBeRemoved is size = 0 now

        // find expired, expired-sequences
        for(std::map<uint32_t, uint32_t>::iterator eit = m_expiredSequences.begin(); eit != m_expiredSequences.end(); ++eit)
        {
            if(eit->second < timestamp) { toBeRemoved.push(eit->first); }
        }

        // remove expired, expired-sequences
        while(toBeRemoved.size() > 0)
        {
            m_expiredSequences.erase(toBeRemoved.front());
            toBeRemoved.pop();
        }

        // request retransmissions
        if(m_retryRequests.size() > 0)
        {
            bool lagging = false;
            for(std::map<uint32_t, RetryRequest_Struct*>::iterator rit = m_retryRequests.begin(); rit != m_retryRequests.end(); ++rit)
            {
                RetryRequest_Struct* rr = rit->second; // get handle on request
                if(rr->nextRequestTime < timestamp)
                {
                    // if second pass, assume lagging
                    if(rr->nextRequestTime != 0) { lagging = true; }
                    else { rr->nextRequestTime = timestamp + m_retryTimeout; }

                    // send actual request
                   Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Connection::update()", "Sending retry request for seq [{}], pktNum [{}]", rr->seqID, rr->pktNum);
                    m_network->sendBuiltin(&m_source, OP_RetransmissionRequest, rr->seqID, rr->pktNum);
                }
            }

            // update connection to lengthen retry wait times
            if(lagging)
            {
               Logger::getInstance().Log(Logs::INFO, Logs::Network, "Connection::update()", "Marking connection [{}] as lagging! m_retryTimeout [{}], m_lastRetry [{}], m_prevLatency [{}]", getIPString(&m_source), m_retryTimeout, m_lastRetry, m_prevLatency);
                //m_retryTimeout *= m_retryTimeout;
                m_retryTimeout *= 2;
                m_isLagging = true;
                m_lastRetry = timestamp;
            }
        }

        return false;
    }

    void Connection::addPacket(Packet** p)
    {
        // dead pointer catch
        if((*p) == nullptr) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "Connection::addPacket()", "Passed ptr->nullptr->? instead of ptr->ptr->obj!"); return; }

        // used to determine usefulness of packet
        bool keepPacket = false;

        // do NOT update estimators on a retransmission reply
        switch((*p)->op_code)
        {
            case OP_RetransmissionReply:
            {
                calculateLatency((*p)->timestamp, Time::getInstance().nowMS(), false);
                keepPacket = true;
                break;
            }
            case OP_RetransmissionAck:
            {
               Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Connection::addPacket()", "OP_RetransmissionAck seqIdent [{}], pktNum [{}]", (*p)->seqIdent, (*p)->pktNum);
                std::multimap<uint32_t, RetryRequest_Struct*>::iterator rit = m_retryRequests.find((*p)->seqIdent);
                if(rit != m_retryRequests.end() && rit->second->pktNum == (*p)->pktNum) { safeDelete(rit->second); m_retryRequests.erase(rit); }
                break;
            }
            case OP_RetransmissionImpossible:
            case OP_Ack:
            {
               Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Connection::addPacket()", "OP_Ack/OP_RetransmissionsImpossible seqIdent [{}], pktNum [{}]", (*p)->seqIdent, (*p)->pktNum);

                // erase all retry requests
                std::multimap<uint32_t, RetryRequest_Struct*>::iterator rit = m_retryRequests.find((*p)->seqIdent);
                while(rit != m_retryRequests.end()) { safeDelete(rit->second); m_retryRequests.erase((*p)->seqIdent); rit = m_retryRequests.find((*p)->seqIdent); }

                // kill off sequence (though this shouldn't be necessary)
                m_sequences.erase((*p)->seqIdent);
                break;
            }
            case OP_KeepAlive:
            {
               Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Connection::addPacket()", "OP_KeepAlive");
                calculateLatency((*p)->timestamp, (*p)->arrivalTime);
                break;
            }
            case OP_ConnectionDisconnect:
            {
               Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Connection::addPacket()", "OP_ConnectionDisconnect");
                m_isClosing = true;
            }
            default:
            {
                // normal packets
                calculateLatency((*p)->timestamp, (*p)->arrivalTime);
                keepPacket = true;
                break;
            }
        }

        if(keepPacket)
        {
            // hand-off or datagram creation
            if((*p)->pktTotal == 1) { directHandOff(*p); } // account for single packets
            else { m_packets.push(*p); } // regular packets

            // setting passed pointer of pointer to nullptr (take ownership)
            *p = nullptr;
        }
        else
        {
            safeDelete((*p));
            //safeDelete(p);
            p = nullptr;
        }
    }

    void Connection::calculateLatency(const uint64_t& pktTime, const uint64_t& arrivalTime, bool updateEstimators /*= true*/)
    {
        // set initial latency
        if(m_latency == -1)
        {
            m_latency = arrivalTime - pktTime;
            for(int i=0;i<10;i++) { m_latencyArray[i] = m_latency; }
            //Logger::getInstance().Log(Logs::DEBUG, "Connection::calculateLatency()", "m_latency first set to {}.", m_latency);
            return;
        }

        // update last arrival
        m_lastArrival = arrivalTime;

        // update estimators, such as latency
        if(updateEstimators)
        {
            m_latencyArray[m_LA] = arrivalTime - pktTime;

            // slot 0
            m_latency = m_latencyArray[0];
            m_latencyStdDev = 0;

            // slots 1-9
            for(int i = 1; i < 10; i++)
            {
                m_latency+=m_latencyArray[i];
                m_latencyStdDev+=(m_latencyArray[i-1] - m_latencyArray[i]);
            }
            m_latency /= 10;
            m_latencyStdDev /= 10;

            /// \TODO: TEST THIS! Review it, then test it again!
            if(m_isLagging)
            {
                if(m_latency <= (m_prevLatency * 0.75f))
                {
                   Logger::getInstance().Log(Logs::INFO, Logs::Network, "Connection::calculateLatency()", "m_latency [{}] is within 25% of m_prevLatency [{}]! ({})", m_latency, m_prevLatency, (int)(m_prevLatency * 0.75f));
                    m_isLagging = false;
                    m_prevLatency = 0;
                    m_lastRetry = 0;
                }
            }

            m_retryTimeout = m_latency + (4 * m_latencyStdDev);
            if(m_retryTimeout < 50) { m_retryTimeout = 50; } // minimum value
            m_LA = (m_LA == 9) ? 0 : m_LA+1;
            /*Logger::getInstance().Log(Logs::DEBUG, "Connection::calculateLatency()", "actualLatency [{} - {} = {}], latency [{}], latencyStdDev [{}], retryTimeout [{}]", arrivalTime, pktTime, (arrivalTime - pktTime), m_latency, m_latencyStdDev, m_retryTimeout);
			Logger::getInstance().Log(Logs::DEBUG, "Connection::calculateLatency()", "latencyArray : {} {} {} {} {} {} {} {} {} {}", m_latencyArray[0], m_latencyArray[1], m_latencyArray[2], m_latencyArray[3],
                                        m_latencyArray[4], m_latencyArray[5], m_latencyArray[6], m_latencyArray[7], m_latencyArray[8], m_latencyArray[9]);*/
        }
    }

    void Connection::keepalive() { m_lastArrival = Time::getInstance().nowMS(); }

    /// Connection protected functions ////////////////////////////////////////

    void Connection::closeConnection()
    {
        // clear retry requests
        for(std::map<uint32_t, RetryRequest_Struct*>::iterator rit = m_retryRequests.begin(); rit != m_retryRequests.end(); ++rit)
        {
            safeDelete(rit->second);
        }
        m_retryRequests.clear();

        // clear all packets
        while(!m_packets.empty()) { safeDelete(m_packets.front()); m_packets.pop(); }

        //safeDelete(m_source);
        m_sequences.clear();
        m_expiredSequences.clear();
        m_buffer = nullptr;
        m_network = nullptr;
        m_isClosed = true;
    }

    // Checks active sequences and expired sequences for existence
    bool Connection::doesExist(uint32_t seq)
    {
        if(m_sequences.find(seq) != m_sequences.end() || m_expiredSequences.find(seq) != m_expiredSequences.end()) { return true; }
        else { return false; }
    }

    /// NetConnection public functions ////////////////////////////////////////

    void copy(NetConnection& dst, const NetConnection& src)
    {
        if(&dst != &src)
        {
            //dst.m_source = src.m_source;
            memcpy(&dst.m_source, &src.m_source, sizeof(struct sockaddr_storage));
            dst.m_ipAddr = src.m_ipAddr;
            dst.m_isClosing = src.m_isClosing;
            dst.m_lastArrival = src.m_lastArrival;
            dst.m_network = src.m_network;
            if(src.m_buffer) { dst.m_buffer = src.m_buffer; }
            dst.m_retryRequests = src.m_retryRequests;
            dst.m_packets = src.m_packets;
            dst.m_sequences = src.m_sequences;
            dst.m_expiredSequences = src.m_expiredSequences;
        }
    }

    NetConnection::NetConnection(Network* net, sockaddr_storage* source, std::string& ipStr, const uint32_t& uniqID, SafeQueue<Datagram*>* buffer)
    {
        m_uniqueID = uniqID;
        m_source = (*source);
        m_buffer = buffer;
        m_ipAddr = getIPString(source);
        m_connType = ConnectionType::NET;
        m_network = net;
        m_lastArrival = Time::getInstance().nowMS();
        m_ipAddr = ipStr;

        // take ownership
        source = nullptr;
    }

    NetConnection::~NetConnection()
    {
        closeConnection();
        m_buffer = nullptr;
    }

    /// NetConnection private functions ///////////////////////////////////////

    void NetConnection::directHandOff(Packet* p)
    {
        Datagram* d = new Datagram(p->op_code, m_uniqueID, p->data, p->dataLength, p->timestamp, this);
        if(d) { m_buffer->push(d); }
        safeDelete(p);
    }

    void NetConnection::sendACK(uint32_t seqID)
    {
        m_network->sendBuiltin(&m_source, OP_Ack, seqID, 0);
    }

    void NetConnection::simpleDatagram(uint16_t OpCode)
    {
        Datagram* d = new Datagram(OpCode, m_uniqueID, nullptr, 0, 0, this);
        if(d) { m_buffer->push(d); }
    }
}
