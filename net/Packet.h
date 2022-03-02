#ifndef PACKET_H_INCLUDED
#define PACKET_H_INCLUDED

#define PACKET_MAX_SIZE 1000
#define PACKET_HEADER_SIZE 39
#define PACKET_MIN_RCV_SIZE 40 /// \TODO: is this actually correct or should it be header size +1)?
#define IDENT_SIZE 4
#define PACKET_DATA_SIZE (PACKET_MAX_SIZE - PACKET_HEADER_SIZE)

#include "common/types.h"
#include "net/RawPacket.h"
#include <string.h>

/*
    References:
        https://en.wikipedia.org/wiki/ANSI_escape_code

*/

namespace CGameEngine
{
    /// \TODO: Convert to true class format with protected functions/values
    /// \TODO: Consider using abort (via assert) to determine packet health/damage
    struct Packet : public RawPacket
    {
        unsigned char* identifier = nullptr; // title identifier string
        uint8_t softwareVersion[3] = { 0, 0, 0 };
        uint16_t op_code = 0; // OP_Codes.h for more info
        uint64_t timestamp = 0; // creation time
        uint32_t seqIdent = 0; // sequence identifier
        uint32_t pktNum = 0; // number in sequence
        uint32_t pktTotal = 0; // total number of packets
        uint32_t totalCRC = 0; // holds crc32 value of whole of data
        uint16_t dataLength = 0; // holds the length of the data buffer
        uint32_t totalLength = 0; // total length of data across packets
        unsigned char* data = nullptr; // actual data

        Packet(uint16_t pSize = PACKET_DATA_SIZE); // "default" ctor
        Packet(unsigned char* d, uint32_t len, const uint64_t& arrival); // ctor from raw data
        Packet(RawPacket& rp); // ctor from raw packet
        Packet(std::string& ident, SoftwareVersion* swv, uint16_t opcode, const uint64_t& timeStamp, uint32_t seqid, uint32_t pktnum, uint32_t pkttotal,
            uint32_t wholeCRC, uint16_t datalength, uint32_t totallength, unsigned char* d); // ctor likely from Network::send()
        Packet(const Packet& p) : Packet(p.dataLength) { copy(*this, p); } // copy ctor
        Packet(Packet&& p) noexcept : Packet(p.dataLength)  { swap(*this, p); } // move ctor
        Packet& operator=(const Packet& p) { copy(*this, p); return *this; } // copy assignment
        Packet& operator=(Packet&& p) noexcept { swap(*this, p); return *this; } // move assignment
        ~Packet() noexcept; // destructor

        const int getSize() const;
        const int getHeaderSize() const;
        const bool isDamaged() const;
        const bool matchesVersion(SoftwareVersion* swv);
        void serializeIn();
        void serializeOut();

        friend void copy(Packet& dst, const Packet& src);
        friend void swap(Packet& dst, Packet& src);

    };
}

#endif // PACKET_H_INCLUDED






























