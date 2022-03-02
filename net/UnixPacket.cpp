#include "UnixPacket.h"


namespace CGameEngine
{
    UnixPacket::UnixPacket(uint16_t psize /*= UNIX_PACKET_DATA_SIZE*/)
        : dataLength(psize), data(new unsigned char[psize])
    {
        memset(data, 0, dataLength);
    }

    UnixPacket::UnixPacket(unsigned char* d, uint32_t len) : RawPacket(d, len)
    {
        if(dataLength+UNIX_PACKET_HEADER_SIZE > UNIX_PACKET_MAX_SIZE) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixPacket::UnixPacket()", "Invalid packet length, received {} though the max is {}!", dataLength, UNIX_PACKET_MAX_SIZE); return; }
        else
        {
            serializeIn();
            if(readPos != pSize) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixPacket::UnixPacket()", "Invalid packet length, post serializeIn(). Expected {}, only have {}!", pSize, readPos); }
        }
    }

    UnixPacket::UnixPacket(uint16_t opCode, const uint64_t& arrival, const uint32_t sender, const uint16_t len, unsigned char* d)
        : op_code(opCode), timestamp(arrival), senderID(sender), dataLength(len), data(new unsigned char[len])
    {
        if(dataLength+UNIX_PACKET_HEADER_SIZE > UNIX_PACKET_MAX_SIZE) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "UnixPacket::UnixPacket(full)", "Invalid packet length, received {} though the max is {}!", dataLength, UNIX_PACKET_MAX_SIZE); return; }

        try
        {
            memset(data, 0, dataLength);
            memcpy(data, d, dataLength);
        }
        catch (...)
        {
           Logger::getInstance().Log(Logs::WARN, Logs::Network, "UnixPacket::UnixPacket(full)", "Damaged packet received, discarding.");
            return;
        }
    }

    UnixPacket::~UnixPacket() noexcept
    {
        safeDeleteArray(data);
    }

    const int UnixPacket::getHeaderSize() const
    {
        int psize = 0;
        psize += sizeof(op_code);
        psize += sizeof(timestamp);
        psize += sizeof(senderID);
        psize += sizeof(dataLength);
        return psize;
    }

    const int UnixPacket::getSize() const
    {
        int psize = getHeaderSize();
        psize += dataLength; // data uchar array
        return psize;
    }

    void UnixPacket::serializeIn()
    {
        // ensure readPos is 0
        readPos = 0;

        op_code = readUInt16();
        timestamp = readUInt64();
        senderID = readUInt32();
        dataLength = readUInt16();
        data = new unsigned char[dataLength];
        readUCharArr(data, dataLength);

        pSize = getSize();
        safeDeleteArray(buffer);
    }

    void UnixPacket::serializeOut()
    {
        // reset buffer and writePos
        safeDeleteArray(buffer);
        writePos = 0;
        pSize = getSize();
        buffer = new unsigned char[pSize];
        memset(buffer, 0, pSize);

        writeUInt16(op_code);
        writeUInt64(timestamp);
        writeUInt32(senderID);
        writeUInt16(dataLength);
        writeUCharArr(data, dataLength);

        trimBuffer();
    }

    void copy(UnixPacket& dst, const UnixPacket& src)
    {
        if(&dst != &src)
        {
            using std::copy;
            dst.op_code = src.op_code;
            dst.timestamp = src.timestamp;
            dst.senderID = src.senderID;
            dst.dataLength = src.dataLength;
            copy(src.data, src.data+src.dataLength, dst.data);
        }
    }

    void swap(UnixPacket& dst, UnixPacket& src)
    {
        if(&dst != &src)
        {
            // enable ADL (not necessary in our case, but good practice)
            using std::swap;

            // by swapping the members of two objects, the two objects are effectively swapped
            safeDeleteArray(dst.data);
            swap(dst.op_code, src.op_code);
            swap(dst.timestamp, src.timestamp);
            swap(dst.senderID, src.senderID);
            swap(dst.dataLength, src.dataLength);
            swap(dst.data, src.data);
        }
    }
}











