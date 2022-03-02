#include "net/Packet.h"


namespace CGameEngine
{
    Packet::Packet(uint16_t pSize/*= PACKET_DATA_SIZE*/) :
        identifier(new unsigned char[IDENT_SIZE]), data(new unsigned char[pSize])
    {
        memset(identifier, 0, IDENT_SIZE);
        memset(data, 0, pSize);
    }

    // constructor from raw data
    Packet::Packet(unsigned char* d, uint32_t len, const uint64_t& arrival) : RawPacket(d, len, arrival)
    {
        if(len > PACKET_MAX_SIZE || len < PACKET_MIN_RCV_SIZE) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "Packet::Packet(uchar*, int)", "Invalid packet length! [{}]", len); return; } // invalid!
        else
        {
            identifier = new unsigned char[IDENT_SIZE];
            memset(identifier, 0, IDENT_SIZE);
            serializeIn();
            //Logger::getInstance().Log(Logs::DEBUG, "Packet::Packet(uchar*, int)", "arrival [{}], creation [{}] (diff {})", arrival, timestamp, (arrival-timestamp));
        }
    }

    Packet::Packet(RawPacket& rp) : Packet(rp.pSize)  // ctor from raw packet
    {
        memcpy(buffer, rp.buffer, rp.pSize);
        serializeIn();

        if(rp.readPos != rp.pSize) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "Packet::Packet(uchar*, int)", "readPos != pSize of RawPacket ({} vs {})", rp.readPos, rp.pSize); }
        else { isDamaged(); }
    }

    // constructor likely from Network::send()
    Packet::Packet(std::string& ident, SoftwareVersion* swv, uint16_t opcode, const uint64_t& timeStamp, uint32_t seqid, uint32_t pktnum, uint32_t pkttotal,
            uint32_t wholeCRC, uint16_t datalength, uint32_t totallength, unsigned char* d) :
                identifier(new unsigned char [IDENT_SIZE]), op_code(opcode), timestamp(timeStamp), seqIdent(seqid), pktNum(pktnum), pktTotal(pkttotal),
                totalCRC(wholeCRC), dataLength(datalength), totalLength(totallength), data(new unsigned char[dataLength])
    {
        if(dataLength+PACKET_HEADER_SIZE > PACKET_MAX_SIZE) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "Packet::Packet(full)", "Invalid packet length! [{}]", dataLength); return; }
        else
        {
            softwareVersion[0] = swv->sw_major;
            softwareVersion[1] = swv->sw_minor;
            softwareVersion[2] = swv->sw_patch;
            try
            {
                memset(identifier, 0, IDENT_SIZE);
                memset(data, 0, dataLength);
                memcpy(identifier, ident.c_str(), IDENT_SIZE);
                memcpy(data, d, dataLength);
            }
            catch (...)
            {
               Logger::getInstance().Log(Logs::WARN, Logs::Network, "Packet::Packet(full)", "Damaged packet received, discarding.");
                return;
            }
        }
    }

    // destructor
    Packet::~Packet() noexcept
    {
        safeDeleteArray(identifier);
        safeDeleteArray(data);
    }

    /// \TODO: Will this fail as it does not account for the RawPacket beneath, or does it even need to?
    const int Packet::getSize() const
    {
        int psize = getHeaderSize();
        psize+=dataLength; // data uchar*
        return psize;
    }

    const int Packet::getHeaderSize() const
    {
        int psize = 0;
        psize+=IDENT_SIZE;
        psize+=sizeof(softwareVersion);
        psize+=sizeof(op_code);
        psize+=sizeof(timestamp);
        psize+=sizeof(seqIdent);
        psize+=sizeof(pktNum);
        psize+=sizeof(pktTotal);
        psize+=sizeof(totalCRC);
        psize+=sizeof(dataLength);
        psize+=sizeof(totalLength);
        return psize;
    }

    const bool Packet::isDamaged() const
    {
        try
        {
            if((pktNum > pktTotal && pktTotal != 0) ||
                dataLength > PACKET_DATA_SIZE ||
                totalLength < dataLength )
            {
               Logger::getInstance().Log(Logs::WARN, Logs::Network, "Packet::isDamaged()", "Packet is damaged.");
               Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Packet::isDamaged()", "dataLength [{}], totalLength [{}], pktNum [{}], pktTotal [{}]", dataLength, totalLength, pktNum, pktTotal);
               Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Packet::isDamaged()", "(pktNum > pktTotal && pktTotal != 0) = {}", (bool)((pktNum > pktTotal && pktTotal != 0)));
               Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Packet::isDamaged()", "dataLength > PACKET_DATA_SIZE = {}", (bool)(dataLength > PACKET_DATA_SIZE));
               Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "Packet::isDamaged()", "totalLength < dataLength  = {}", (bool)(totalLength < dataLength));
                return true;
            }
        }
        catch (...)
        {
           Logger::getInstance().Log(Logs::CRIT, Logs::Network, "Packet::isDamaged()", "'catch' hit, Packet is definitely damaged!");
            return true;
        }

        // got this far, must be safe
        return false;
    }

    const bool Packet::matchesVersion(SoftwareVersion* swv)
    {
        // return false if anything does not match
        if( swv->sw_major != softwareVersion[0] ||
            swv->sw_minor != softwareVersion[1] ||
            swv->sw_patch != softwareVersion[2])
        { return false; }
        else { return true; }
    }

    void Packet::serializeIn()
    {
        // ensure readPos is 0
        readPos = 0;

        readUCharArr(identifier, IDENT_SIZE);
        softwareVersion[0] = readUInt8();
        softwareVersion[1] = readUInt8();
        softwareVersion[2] = readUInt8();
        op_code = readUInt16();
        timestamp = readUInt64();
        seqIdent = readUInt32();
        pktNum = readUInt32();
        pktTotal = readUInt32();
        totalCRC = readUInt32();
        dataLength = readUInt16();
        totalLength = readUInt32();
        safeDeleteArray(data);
        data = new unsigned char[dataLength];
        readUCharArr(data, dataLength);

        pSize = getSize();
        safeDeleteArray(buffer);
    }

    void Packet::serializeOut()
    {
        safeDeleteArray(buffer);
        writePos = 0; // reset
        pSize = getSize();
        buffer = new unsigned char[pSize];
        memset(buffer, 0, pSize);

        writeUCharArr(identifier, IDENT_SIZE);
        writeUInt8(softwareVersion[0]);
        writeUInt8(softwareVersion[1]);
        writeUInt8(softwareVersion[2]);
        writeUInt16(op_code);
        writeUInt64(timestamp);
        writeUInt32(seqIdent);
        writeUInt32(pktNum);
        writeUInt32(pktTotal);
        writeUInt32(totalCRC);
        writeUInt16(dataLength);
        writeUInt32(totalLength);
        writeUCharArr(data, dataLength);

        trimBuffer();
    }

    void copy(Packet& dst, const Packet& src)
    {
        if(&dst != &src)
        {
            using std::copy;

            copy(src.identifier, src.identifier+IDENT_SIZE, dst.identifier);
            dst.op_code = src.op_code;
            dst.timestamp = src.timestamp;
            dst.seqIdent = src.seqIdent;
            dst.pktNum = src.pktNum;
            dst.pktTotal = src.pktTotal;
            dst.totalCRC = src.totalCRC;
            dst.dataLength = src.dataLength;
            dst.totalLength = src.totalLength;
            copy(src.data, src.data+src.dataLength, dst.data);
        }
    }

    // source: https://stackoverflow.com/a/3279550/6845246
    void swap(Packet& dst, Packet& src) // nothrow
    {
        if(&dst != &src)
        {
            // enable ADL (not necessary in our case, but good practice)
            using std::swap;

            // by swapping the members of two objects, the two objects are effectively swapped
            safeDeleteArray(dst.identifier);
            safeDeleteArray(dst.data);
            swap(dst.identifier, src.identifier);
            swap(dst.op_code, src.op_code);
            swap(dst.timestamp, src.timestamp);
            swap(dst.seqIdent, src.seqIdent);
            swap(dst.pktNum, src.pktNum);
            swap(dst.pktTotal, src.pktTotal);
            swap(dst.totalCRC, src.totalCRC);
            swap(dst.dataLength, src.dataLength);
            swap(dst.totalLength, src.totalLength);
            swap(dst.data, src.data);
        }
    }
}
