#ifndef PACKETPAIR_H_INCLUDED
#define PACKETPAIR_H_INCLUDED

#include "common/types.h"
#include <sys/socket.h> // sockaddr_storage
#include "net/Packet.h"


namespace CGameEngine
{
    struct PacketPair
    {
            PacketPair(Packet** p, struct sockaddr_storage s) : sas()
            {
                // dead end
                if(*p == nullptr) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "PacketPair::PacketPair()", "No packet passed!"); return; }

                // take ownership
                pkt = (*p); *p = nullptr;
                memcpy(&sas, &s, sizeof(struct sockaddr_storage));
                if(!pkt) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "PacketPair::PacketPair()", "We were created without a packet!"); }
            }
            PacketPair() {} // default ctor
            ~PacketPair() noexcept {}
            PacketPair(const PacketPair& r) { copy(*this, r); } // copy constructor
            PacketPair(PacketPair&& r)  { swap(*this, r); } // move constructor
            PacketPair& operator=(const PacketPair& p) { copy(*this, p); return *this; } // copy assignment
            PacketPair& operator=(PacketPair&& p) noexcept { swap(*this, p); return *this; } // move assignment

            void destroy() noexcept { safeDelete(pkt); }

            friend void copy(PacketPair& dst, const PacketPair& src) // nothrow
            {
                if(&dst != &src)
                {
                    // enable ADL (not necessary in our case, but good practice)
                    using std::copy;
                    dst.pkt = src.pkt;
                    memcpy(&dst.sas, &src.sas, sizeof(struct sockaddr_storage));
                }
            }
            friend void swap(PacketPair& dst, PacketPair& src) // nothrow
            {
                if(&dst != &src)
                {
                    // enable ADL (not necessary in our case, but good practice)
                    using std::swap;
                    swap(dst.pkt, src.pkt);
                    swap(dst.sas, src.sas);
                    src.pkt = nullptr;
                }
            }

            // resources
            Packet* pkt = nullptr;
            struct sockaddr_storage sas;
    };
}

#endif // PACKETPAIR_H_INCLUDED
