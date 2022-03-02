#ifndef NETWORK_H
#define NETWORK_H

#include "common/types.h"
#include "common/SafeQueue.h"
#include "common/SafeUnorderedMap.h"
#include "common/CRC32.h"
#include "common/util.h"
#include "net/Builtin_OP_Codes.h"
#include "net/Builtin_Structs.h"
#include "net/PacketPair.h"
#include "net/PacketSequence.h"
#include "net/Datagram.h"
#include "net/NetSocket.h" // Socket
#include "net/Connection.h" // NetConnection
#include "net/net_util.h"
#include "srv/Time.h"
//#include <event2/event.h> // libevent 2.1.8
#include <sys/poll.h> // poll()
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cmath>


/*
    Resolve IP to Hostname : http://stackoverflow.com/questions/10564525/resolve-ip-to-hostname
    getnameinfo : http://man7.org/linux/man-pages/man3/getnameinfo.3.html
    getaddinfo : http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
    libsocket : https://github.com/dermesser/libsocket
    unix socket : https://github.com/zappala/socket-programming-examples-c
    inet socket : https://github.com/haydenjameslee/UDP-Socket-Hello-World

    //// Special Signal Hashes ////
    'GAME Connection Init' (HELO) : 5b02099aa0a9d5a29403c32208c04a73
    'GAME Accepting Connections' (HELO Server) : 16d63a14a61b7d80e879ef112cf2a9bc
    'GAME Acknowledged' (ACK) : ece08acfd688c2afe82e5fe9a1289516
    'GAME' : d0c8fe1180acbc4f984abfd98a1c7f12

    Ref:
        http://cgi.di.uoa.gr/~ad/k24/set006.pdf
            Best lazy reference possible (although dated)


        https://stackoverflow.com/questions/8590332/difference-between-sizeof-and-strlen-in-c
            strlen() is used to get the length of an array of chars / string.
            sizeof() is used to get the actual size of any type of data in bytes.
        https://stackoverflow.com/questions/22822145/udp-transfer-maintaining-network-byte-order
            Packet ordering posts
        https://stackoverflow.com/questions/13095513/what-is-the-difference-between-memcmp-strcmp-and-strncmp-in-c
            strcmp() compares null-terminated C strings
            strncmp() compares at most N characters of null-terminated C strings
            memcmp() compares binary byte buffers of N bytes
        https://en.wikipedia.org/wiki/Busy_waiting
            READ ON SPIN LOCKING!!
        http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#datagram
            datagram networking
        https://www.codeproject.com/Articles/275715/Real-time-communications-over-UDP-protocol-UDP-RT
            VERSIONING IN HEADER
        https://commandcenter.blogspot.fr/2012/04/byte-order-fallacy.html
            Byte Order Fallacy (TL;DR: don't worry about your platform native order,
                                all that counts is the byte order of the stream your
                                are reading from, and you better hope it's well defined.)
        https://juanchopanzacpp.wordpress.com/2013/02/26/concurrent-queue-c11/
            Concurrent queues, or how to make thread-safe std::queue implementations

        http://preshing.com/20150324/safe-bitfields-in-cpp/
            Bitfields via preprocessor additions
        http://blog.codef00.com/2014/12/06/portable-bitfields-using-c11/
            Portable bitfields in C++11
        http://www.masterraghu.com/subjects/np/introduction/unix_network_programming_v1.3/ch11lev1sec6.html
            getaddrinfo() and ai_flags
        https://github.com/jasonish/libevent-examples/tree/master/echo-server
        http://libevent.org/
            ::: libevent 2 information :::
            *   See "Reinitializing an event_base after fork()" - http://www.wangafu.net/~nickm/libevent-book/Ref2_eventbase.html
            *   udp example - https://github.com/jasonish/libevent-examples/blob/master/echo-server/libevent_echosrv2.c
        https://rubentorresbonet.wordpress.com/2014/08/19/c-11-stop-polling-long-live-stdcondition_variable/
            GLORIOUS POSTING



    *** getaddrinfo() ai_flags ***
    AI_PASSIVE      The caller will use the socket for a passive open.
    AI_CANONNAME    Tells the function to return the canonical name of the host.
    AI_NUMERICHOST  Prevents any kind of name-to-address mapping; the hostname argument must be an address string.
    AI_NUMERICSERV  Prevents any kind of name-to-service mapping; the service argument must be a decimal port number string. (i.e. "http")
    AI_V4MAPPED     If specified along with an ai_family of AF_INET6, then returns IPv4-mapped IPv6 addresses corresponding to A records if there are no available AAAA records.
    AI_ALL          If specified along with AI_V4MAPPED, then returns IPv4-mapped IPv6 addresses in addition to any AAAA records belonging to the name.
    AI_ADDRCONFIG   Only looks up addresses for a given IP version if there is one or more interface that is not a loopback interface configured with a IP address of that version.

    *** poll revents ***
    POLLIN      1
    POLLPRI     2
    POLLOUT     4
    POLLERR     8
    POLLHUP     10
    POLLNVAL    20
*/

/// \TODO: Heavily test, break, and retest packet retransmissions
/// \TODO: Re-evaluate poll() with or without spin locking
/// \TODO: Remove/cleanup/something with TCP remnants

namespace NetworkType { enum FORMS { NONE, Base, Server, Client, InternalServer, InternalClient, CustomServer, CustomClient, Peer, END }; }

namespace CGameEngine
{
    struct OutboundPacket
    {
    	OutboundPacket(sockaddr_storage* destination, unsigned char* packetData, uint32_t packetSize)
    		: addr(destination), data(packetData), pSize(packetSize) { }
    	~OutboundPacket() { addr = nullptr; data = nullptr; }
    	sockaddr_storage* addr = nullptr;

    	int fd = -1;
    	unsigned char* data = nullptr;
        uint32_t pSize = 0;
    };

    // A = 10.0.0.0/8
    // B = 172.16.0.0/16
    // C = 192.168.1.0/24
    //namespace InternalNetworkClass { enum FORMS { NONE, A, B, C }; }

    class Network
    {
        public:
            Network() {}
            Network(SoftwareVersion* swv, uint16_t port, SafeQueue<Datagram*>* dgbuff, std::string hostname = "");
            virtual ~Network();
            virtual void updateLoop() = 0;
            virtual bool shutdownSockets() = 0;
            virtual bool isPacketValid(Packet* p, sockaddr_storage* sender = nullptr) = 0;
            virtual void listenLoop();
            virtual void sendLoop();
            void stop();
            void generateAddress(const std::string addr, uint16_t port, addrinfo** dst);
            void generateInternalAddress(uint16_t port, addrinfo** dst, std::string hostname = "");
            const bool& isAccepting() const { return m_netListening; } // is network accepting outside connections
            const bool& isActive() const { return m_isActive; } // is network object functioning
            bool isPortOpen(uint16_t port); // network

            void broadcast(uint16_t port, uint16_t opCode, unsigned char** data, uint32_t dataLength);
            void send(sockaddr_storage* addr, uint16_t opCode, unsigned char** data, uint32_t dataLength);
            void sendBuiltin(sockaddr_storage* addr, uint16_t opCode, uint32_t seqID = 0, uint32_t pktNum = 0);
            void sendRetryResponse(sockaddr_storage* addr, Packet* p);
            void sendSimple(sockaddr_storage* addr, uint16_t opCode);
            void setAccepting(bool val = true);
            void setTitle(std::string str);
            void setUniqueID(uint32_t& id) { m_uniqueID = id; }
            //void setEventConditionVariable(std::condition_variable* cv) { m_userCV = cv; }

            SafeQueue<Datagram*>* getDatagramBuffer() { return m_datagramBuffer; }
            const uint8_t& getNetworkType() const { return m_networkType; }
            const uint32_t& getUniqueID() const { return m_uniqueID; }
            const std::string getIPAddress() const { return getIPString(m_srcAddress); }

            // thread starters
            static void startListenLoop(Network* n) { n->listenLoop(); }
            static void startSendLoop(Network* n) { n->sendLoop(); }
            static void startUpdateLoop(Network* n) { n->updateLoop(); }

        protected:
            virtual bool initSockets();
            virtual uint32_t& getSequenceID();
            bool m_isConnected = false; // TCP ONLY
            bool m_isActive = false; // used to signify a working network object
            bool m_isTCP = false; // TCP ONLY
            bool m_netListening = false; // used to delay connections during server boot
            int m_pollTimeout = 3000;
            uint8_t m_networkType = NetworkType::Base;
            uint16_t m_srcPort = 0; // listening port
            uint32_t m_seqID = 1; // 0 - 1bil for srv, 1bil - 4bil for client (1k IDs per client)
            uint32_t m_uniqueID = 0;
            std::string m_identifier = "";
            std::string m_title = "";
            std::string m_srcHostname = "";
            addrinfo* m_bcAddress = nullptr; // broadcast address
            addrinfo* m_srcAddress = nullptr; // this device's address
            NetSocket* m_socket = nullptr; // this device's default socket (0.0.0.0 equivalent)
            SafeQueue<Datagram*>* m_datagramBuffer = nullptr; // default datagram buffer
            SafeQueue<OutboundPacket> m_sendQueue;
            SafeQueue<PacketPair> m_packetBuffer;
            SoftwareVersion* m_version = nullptr;
            std::condition_variable m_sendCV;
            std::condition_variable m_updateCV;
            //std::condition_variable* m_userCV = nullptr;
            std::thread* m_listenThread = nullptr; // active thread
            std::thread* m_sendThread = nullptr; // idle thread
            std::thread* m_updateThread = nullptr; // semi-active thread
            SafeUnorderedMap<uint32_t, StoredSequence*> m_storedSequences;
            SafeUnorderedMap<int, std::pair<NetSocket*, SafeQueue<Datagram*>*>> m_socketPairs; // FD and its associated datagram buffer

            // near static values for sequence ID generation
            uint32_t MIN_SEQ_ID = 1;
            uint32_t MAX_SEQ_ID = 2000000000;
            std::chrono::milliseconds HEARTBEAT_INTERVAL = std::chrono::milliseconds(250);
    };
}

#endif // NETWORK_H
