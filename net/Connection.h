#ifndef NETCONNECTION_H
#define NETCONNECTION_H

#include "common/types.h"
#include "net/Socket.h"
#include "net/Network.h"
#include "common/SafeQueue.h"
#include <map>
#include <string>

/*
    Assuming minimum connection speed of 30kbps, which is 40 FULL packets per second

    ref:
        http://www.masterraghu.com/subjects/np/introduction/unix_network_programming_v1.3/ch22lev1sec5.html
*/

/// \TODO: Review passing netcon/unixcon on datagrams to correctly indicate the source!
/// \TODO: Review better way to get timestamped comparisons (ntp) from src send time to dst arrival time

namespace ConnectionType { enum FORMS { BASE, NET }; }

namespace CGameEngine
{
    class Network;
    class Connection
    {
        friend class PacketSequence;

        public:
            Connection() {}
            virtual ~Connection() { closeConnection(); }
            bool update(uint64_t timestamp);
            const bool& isClosed() const { return m_isClosed; }
            const int& getConnectionType() const { return m_connType; }
            const uint32_t& getUniqueID() const { return m_uniqueID; }
            void addPacket(Packet** p);
            void calculateLatency(const uint64_t& pktTime, const uint64_t& arrivalTime, bool updateEstimators = true);
            void keepalive();
            void setDatagramBuffer(SafeQueue<Datagram*>* buffer) { m_buffer = buffer; }
            void setClosed(bool val = true) { m_isClosing = val; }
            void setUniqueID(uint32_t& id) { m_uniqueID = id; }
            sockaddr_storage* getSource() { return &m_source; }

        protected:
            void closeConnection();
            bool doesExist(uint32_t seq);
            virtual void simpleDatagram(uint16_t OpCode) = 0;
            virtual void directHandOff(Packet* p) = 0;

            bool m_isClosed = false;
            bool m_isClosing = false; // open for all types of packets
            bool m_isLagging = false;
            int m_connType = ConnectionType::BASE; // base
            int m_prevLatency = 0; // 'snapshot' of m_latency
            int m_latency = -1; // MS
            int m_latencyStdDev = 0; // MS
            int m_retryTimeout = 0; // MS
            int m_latencyArray[10] = {0};
            uint8_t m_LA = 0;
            uint32_t m_uniqueID = 0; // sender's uniqID
            uint64_t m_lastArrival = 0; // MS timestamp
            uint64_t m_lastRetry = 0; // MS timestamp
            sockaddr_storage m_source;
            Network* m_network = nullptr;
            SafeQueue<Datagram*>* m_buffer = nullptr;
            std::multimap<uint32_t, RetryRequest_Struct*> m_retryRequests;
            SafeQueue<Packet*> m_packets;
            std::map<uint32_t, PacketSequence> m_sequences;
            std::map<uint32_t, uint32_t> m_expiredSequences;
    };

    /// \TODO: Add getaddrinfo to store "complete" inet info
    /// \TODO: Break out into separate file(s)
    class NetConnection : public Connection
    {
        friend class PacketSequence;
        public:
            NetConnection(Network* net, sockaddr_storage* source, std::string& ipStr, const uint32_t& uniqID, SafeQueue<Datagram*>* buffer);
            ~NetConnection();
            friend void copy(NetConnection& dst, const NetConnection& src);
            NetConnection(const NetConnection& ps) { copy(*this, ps); }
            NetConnection& operator=(const NetConnection& ps) { copy(*this, ps); return *this; }
            const std::string& getIPAddr() const { return m_ipAddr; }

        private:
            std::string m_ipAddr = "";
            void directHandOff(Packet* p) override;
            void sendACK(uint32_t seqID);
            void simpleDatagram(uint16_t OpCode) override;
    };
}

#endif // NETCONNECTION_H
