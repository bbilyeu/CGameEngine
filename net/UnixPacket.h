#ifndef UDPPACKET_H
#define UDPPACKET_H

#define UNIX_PACKET_MAX_SIZE 65000
#define UNIX_PACKET_HEADER_SIZE 16
#define UNIX_PACKET_DATA_SIZE (UNIX_PACKET_MAX_SIZE - UNIX_PACKET_HEADER_SIZE)

#include "common/types.h"
#include "net/RawPacket.h"

namespace CGameEngine
{
    struct UnixPacket : public RawPacket
    {
        uint16_t op_code = 0;
        uint64_t timestamp = 0; // passed from Packet or Connection
        uint32_t senderID = 0; // sender's uniq ID
        uint16_t dataLength = 0;
        unsigned char* data = nullptr;

        UnixPacket(uint16_t pSize = UNIX_PACKET_DATA_SIZE); // default ctor
        UnixPacket(unsigned char* d, uint32_t len);
        UnixPacket(uint16_t opCode, const uint64_t& arrival, const uint32_t sender, const uint16_t len, unsigned char* d);
        UnixPacket(const UnixPacket& p) : UnixPacket(p.dataLength) { copy(*this, p); } // copy ctor
        UnixPacket(UnixPacket&& p) noexcept : UnixPacket(p.dataLength)  { swap(*this, p); } // move ctor
        UnixPacket& operator=(const UnixPacket& p) { copy(*this, p); return *this; } // copy assignment
        UnixPacket& operator=(UnixPacket&& p) noexcept { swap(*this, p); return *this; } // move assignment
        virtual ~UnixPacket() noexcept;

        const int getHeaderSize() const;
        const int getSize() const;
        void serializeIn();
        void serializeOut();

        friend void copy(UnixPacket& dst, const UnixPacket& src);
        friend void swap(UnixPacket& dst, UnixPacket& src);
    };
}

#endif // UDPPACKET_H
