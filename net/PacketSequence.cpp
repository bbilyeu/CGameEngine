#include "net/PacketSequence.h"
#include "net/Connection.h"
#include "common/CRC32.h"
#include <algorithm>


namespace CGameEngine
{
    PacketSequence::PacketSequence(uint32_t seq_ID, int numPackets, const uint64_t& arrivalTimestamp, Connection* parentConn) :
        m_seqID(seq_ID), m_numberPackets(numPackets)
    {
        m_parentConn = parentConn;
        m_originTimestamp = arrivalTimestamp;

        int minLatency = (m_parentConn->m_latency >= 20) ? m_parentConn->m_latency : 20;
        uint64_t timeToCompletion = minLatency * m_numberPackets;
        m_retryThreshold = m_originTimestamp + (timeToCompletion * 0.2f);
        m_hardExpiration = m_originTimestamp + (timeToCompletion * 1.2f);
       Logger::getInstance().Log(Logs::INFO, Logs::Network,
                "PacketSequence::PacketSequence()", "seqID [{}], numPackets [{}], origin [{}], ttC [{}], retryThreshold [{}], hardExpiration [{}]",
                m_seqID, m_numberPackets, m_originTimestamp, timeToCompletion, m_retryThreshold, m_hardExpiration);
    }

    PacketSequence::~PacketSequence()
    {
        for(int i = 0; i < m_packets.size(); i++) { safeDelete(m_packets[i]); }
        m_packets.clear();
        m_missing.clear();
        m_parentConn = nullptr;
    }

    void copy(PacketSequence& dst, const PacketSequence& src)
    {
        if(&dst != &src)
        {
            dst.m_seqID = src.m_seqID;
            dst.m_numberPackets = src.m_numberPackets;
            dst.m_hardExpiration = src.m_hardExpiration;
            dst.m_retryThreshold = src.m_retryThreshold;
            dst.m_packets.setVector(src.m_packets);
            dst.m_parentConn = src.m_parentConn;
        }
    }

    bool PacketSequence::update(const uint64_t& time)
    {
        if(m_hardExpiration < time && m_hardExpiration != -1 && !m_parentConn->m_isLagging)
        {
			Logger::getInstance().Log(Logs::DEBUG, "PacketSequence::update()",
                    "seqID [{}] is closing. hardExpiration [{}] vs time [{}], numReceived [{}] vs \
                    total [{}], is parentConn lagging [{}]", m_seqID, m_hardExpiration, time, m_packets.size(), m_numberPackets);
            return true;
        }
        else if(m_packets.size() <= 1 || !m_parentConn) { return false; }

        // pull down current timeout
        m_retryTimeout = m_parentConn->m_retryTimeout;

        // this sequence has all of it's packets
        if(m_packets.size() == m_numberPackets)
        {
           Logger::getInstance().Log(Logs::DEBUG, "PacketSequence::update()", "Assembling entire sequence [seqID {}]", m_seqID);
            Packet* p = m_packets[0];
            int pos = 0;
            int length = p->totalLength;

            std::sort(m_packets.begin(), m_packets.end(), pktCompare);

            unsigned char* data = new unsigned char[length];
            memset(data, 0, length);

            // assemble data
            for(unsigned int d = 0; d < m_packets.size(); d++)
            {
                //std::cout<<"\033[1mPacketSequence::update\033[0m - Assembling pktNum ["<<(m_packets[d]->pktNum + 1)<<"] of ["<<m_packets[d]->pktTotal<<"] \tPos ["<<pos<<"] of ["<<length<<"] (size "<<m_packets[d]->dataLength<<") (pos should be "<<PACKET_DATA_SIZE*d<<")\n";
                memcpy(data+pos, m_packets[d]->data, m_packets[d]->dataLength);
                pos += m_packets[d]->dataLength;
            }
           Logger::getInstance().Log(Logs::DEBUG, "PacketSequence::update()", "Assembled! Pos [{}] of [{}]", pos, length);

            // crc check it
            uint32_t crc = CRC32::create(data, length);

            // if crc check passes
            if(crc == p->totalCRC)
            {
                if(m_parentConn->getConnectionType() == ConnectionType::NET)
                {
                    Datagram* d = new Datagram(p->op_code, m_parentConn->getUniqueID(), data, p->dataLength, m_originTimestamp, (NetConnection*)m_parentConn);
                    if(d) { m_parentConn->m_buffer->push(d); }
                }
            }
            else
            {
                std::string order = " (order:";
                for(unsigned int i = 0; i < m_packets.size(); i++) { order+= " " + std::to_string(m_packets[i]->pktNum); }
                order+= ")";
               Logger::getInstance().Log(Logs::WARN, Logs::Network, "PacketSequence::update()", "CRC does not match! Damaged Packet Sequence ({})! Expected [{}], received [{}]! {}", m_seqID, crc, p->totalCRC, order);
            }

            // cleanup
            safeDeleteArray(data);

            // destroy the packet sequence
           Logger::getInstance().Log(Logs::INFO, Logs::Network, "PacketSequence::update()", "seqID [{}] is closing. m_numberPackets = m_packets.size() [{} = {}]", m_seqID, m_numberPackets, m_packets.size());
            if(m_parentConn->m_connType == ConnectionType::NET) { static_cast<NetConnection*>(m_parentConn)->sendACK(m_seqID); }
            return true;
        }
        else if(m_retryThreshold < time)
        {
            std::vector<int> missing;

            // REF: https://stackoverflow.com/questions/379383/finding-gaps-in-sequence-of-numbers
            std::sort(m_packets.begin(), m_packets.end(), pktCompare);

            // iterator through to find missing numbers
            int offset = 0;
            for(unsigned int i = 0; i < m_packets.size(); i++)
            {
                Packet* p = m_packets[i];
                if(p->pktNum != i+offset)
                {
                   Logger::getInstance().Log(Logs::DEBUG, "PacketSequence::update()", "Seeing [{}], expected [{}] (retry [{}] vs time [{}], offset [{}])", p->pktNum, (i+offset), m_retryThreshold, time, offset);
                    missing.push_back(i);
                    offset++;
                }
            }

            // actually processing new retry requests
            if(missing.size() > 0)
            {
                // increment retry threshold
                int increment = (m_parentConn->m_latency * 2.2f);
               Logger::getInstance().Log(Logs::DEBUG, "PacketSequence::update()", "Increasing threshold by [{}] to [{}] (was {}).", increment, (m_retryThreshold+increment), m_retryThreshold);
                m_retryThreshold += increment;

                // request retry if new
                for(unsigned int i = 0; i < missing.size(); i++)
                {
                    if(!m_missing.contains(missing[i]))
                    {
                        /// \TODO: Re-evaluate process for requesting retries
                       Logger::getInstance().Log(Logs::INFO, Logs::Network, "PacketSequence::update()", "Requesting retry for seqID [{}], pktNum [{}]", m_seqID, missing[i]);
                        RetryRequest_Struct* rr = new RetryRequest_Struct(m_seqID, missing[i], 0);
                        m_parentConn->m_retryRequests.insert( std::make_pair(m_seqID, rr) );
                        rr = nullptr; // let Connection "own" it
                        m_missing.push_back(missing[i]);
                    }
                }
            }

            // clean up missing numbers vec
            missing.clear();
        }

        // not quite dead yet
        return false;
    }

    void PacketSequence::addPacket(Packet** p, const uint64_t& arrivalTimestamp)
    {
        // add to vector
        m_packets.push_back(*p);

        // if "earlier" packet arrives, update sequence stats
        if(arrivalTimestamp < m_originTimestamp)
        {
            uint64_t diff = arrivalTimestamp - m_originTimestamp;
            m_originTimestamp = arrivalTimestamp;
            m_retryThreshold -= diff;
            m_hardExpiration -= diff;
        }

        // take ownership
        *p = nullptr;
    }


    /// StoredSequence public functions ///////////////////////////////////////

    StoredSequence::StoredSequence(uint32_t seq_ID, int numPackets, const uint64_t& timestamp, std::queue<Packet*> pktQ) : m_seqID(seq_ID), m_numberPackets(numPackets), m_originTimestamp(timestamp)
    {
        m_expiration = m_originTimestamp + 3000; // +3 sec
        //m_packets.reserve(m_numberPackets);

       Logger::getInstance().Log(Logs::INFO, Logs::Network, "StoredSequence::StoredSequence()", "Creating for [{}], [{}] packets, timestamp [{}], expiration [{}]", m_seqID, m_numberPackets, m_originTimestamp, m_expiration);

        // copy from queue to vec
        while(pktQ.size() > 0)
        {
            m_packets.push_back(pktQ.front());
            pktQ.pop();
        }
    }

    StoredSequence::~StoredSequence()
    {
       Logger::getInstance().Log(Logs::INFO, Logs::Network, "StoredSequence::~StoredSequence()", "SeqID [{}] is being destroyed! ({} packets)", m_seqID, m_packets.size());
        while(m_packets.size() > 0)
        {
            safeDelete(m_packets.back());
            m_packets.pop_back();
        }
    }

    void copy(StoredSequence& dst, const StoredSequence& src)
    {
        if(&dst != &src)
        {
            dst.m_seqID = src.m_seqID;
            dst.m_numberPackets = src.m_numberPackets;
            dst.m_originTimestamp = src.m_originTimestamp;
            dst.m_expiration = src.m_expiration;
            dst.m_packets.setVector(src.m_packets);
        }
    }

    void swap(StoredSequence& dst, StoredSequence& src)
    {
        if(&dst != &src)
        {
            std::swap(dst.m_seqID, src.m_seqID);
            std::swap(dst.m_numberPackets, src.m_numberPackets);
            std::swap(dst.m_originTimestamp, src.m_originTimestamp);
            std::swap(dst.m_expiration, src.m_expiration);
            std::swap(dst.m_packets, src.m_packets);
        }
    }
}
