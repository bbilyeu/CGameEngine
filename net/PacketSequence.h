#ifndef PACKETSEQUENCE_H
#define PACKETSEQUENCE_H

#include "common/types.h"
#include "net/Packet.h"
#include "common/SafeVector.h"
#include <queue>

namespace CGameEngine
{
    class Connection;
    class Packet;

    /// \NOTE: PacketSequence is created with the ARRIVAL timestamp, not origin's timestamp
    class PacketSequence
    {
        public:
            PacketSequence(uint32_t seq_ID, int numPackets, const uint64_t& arrivalTimestamp, Connection* parentConn);
            ~PacketSequence();
            friend void copy(PacketSequence& dst, const PacketSequence& src);
            PacketSequence(const PacketSequence& ps) { copy(*this, ps); }
            PacketSequence& operator=(const PacketSequence& ps) { copy(*this, ps); return *this; }
            bool update(const uint64_t& time);
            void addPacket(Packet** p, const uint64_t& arrivalTimestamp);

        private:
            bool m_hasCustomTimeout = false;
            int m_numberPackets = 0;
            uint32_t m_seqID = 0;
            uint64_t m_originTimestamp = 0; // time of creation
            uint32_t m_hardExpiration = 0; // time to forcefully close the packet sequence
            uint32_t m_retryThreshold = 0; // time to start checking for missed (time val)
            uint32_t m_retryTimeout = 0; // time to wait for reply (MS)
            SafeVector<Packet*> m_packets;
            SafeVector<int> m_missing;
            Connection* m_parentConn = nullptr;
            static bool pktCompare(Packet* a, Packet* b) { return (a->pktNum < b->pktNum); }

            // min packets per second
            const int MIN_PPS = 40;
    };

    class StoredSequence
    {
        public:
            StoredSequence(uint32_t seq_ID, int numPackets, const uint64_t& timestamp, std::queue<Packet*> pktQ);
            ~StoredSequence();
            StoredSequence(const StoredSequence& ss) { copy(*this, ss); } // copy ctor
            StoredSequence(StoredSequence&& p) noexcept { swap(*this, p); } // move ctor
            StoredSequence& operator=(const StoredSequence& ss) { copy(*this, ss); return *this; } // copy assignment
            StoredSequence& operator=(StoredSequence&& ss) noexcept { swap(*this, ss); return *this; } // move assignment

            friend void copy(StoredSequence& dst, const StoredSequence& src);
            friend void swap(StoredSequence& dst, StoredSequence& src);

            bool isExpired(uint64_t time) { return (m_expiration <= time); }
            const uint32_t getSeqID() const { return m_seqID; }
            Packet* getPacketByNumber(unsigned int val) { return (val < m_packets.size()) ? m_packets[val] : nullptr; }

        private:
            uint32_t m_seqID = 0;
            int m_numberPackets = 0;
            uint64_t m_originTimestamp = 0;
            uint64_t m_expiration = 0;
            SafeVector<Packet*> m_packets;
    };
}

#endif // PACKETSEQUENCE_H
