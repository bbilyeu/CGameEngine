#ifndef DATAGRAM_H_INCLUDED
#define DATAGRAM_H_INCLUDED

#include "common/types.h"
#include <stdio.h>

namespace CGameEngine
{
    class NetConnection;

    struct Datagram
    {
        uint16_t op_code = 0;
        uint32_t senderUniqID = 0;
        uint64_t timestamp = 0;
        int dataLength = 0;
        unsigned char* data = nullptr;
        NetConnection* netCon = nullptr; // save the network connection source (if applicable)

        Datagram() {}
        Datagram(uint16_t opCode, int bufferSize) : op_code(opCode), dataLength(bufferSize), data(new unsigned char[dataLength]) {}
        Datagram(uint16_t opCode, uint32_t uniqID, uint64_t arrival = 0) : op_code(opCode), senderUniqID(uniqID), timestamp(arrival) {}
        Datagram(uint16_t opCode, uint32_t uniqID, unsigned char* d, int len, uint64_t arrival = 0, NetConnection* nc = nullptr) :
            op_code(opCode), senderUniqID(uniqID), dataLength(len)
        {
            safeDeleteArray(data);
            data = new unsigned char[len];
            memcpy(data, d, len);
            timestamp = arrival;
            netCon = nc;
        }

        Datagram(const Datagram& dg) : Datagram(dg.op_code, dg.dataLength) { copy(*this, dg); } // copy constructor
        Datagram(Datagram&& dg) noexcept : Datagram(dg.op_code, dg.dataLength) { swap(*this, dg); } // move constructor
        Datagram& operator=(const Datagram& dg) { copy(*this, dg); return *this; } // copy assignment
        Datagram& operator=(Datagram& dg) noexcept { swap(*this, dg); return *this; } // move assignment
        friend void copy(Datagram& dst, const Datagram& src)
        {
            if(&dst != &src)
            {
                dst.op_code = src.op_code;
                dst.timestamp = src.timestamp;
                dst.dataLength = src.dataLength;
                std::copy(src.data, src.data + src.dataLength, dst.data);
                dst.netCon = src.netCon;
            }
        }
        friend void swap(Datagram& dst, Datagram& src)
        {
            if(&dst != &src)
            {
                using std::swap;
                swap(dst.op_code, src.op_code);
                swap(dst.timestamp, src.timestamp);
                swap(dst.dataLength, src.dataLength);
                swap(dst.data, src.data); //src.data = nullptr;
                swap(dst.netCon, src.netCon); //src.netCon = nullptr;
            }
        }

        ~Datagram() noexcept
        {
            safeDeleteArray(data);
            netCon = nullptr;
            op_code = 0;
            timestamp = 0;
        }
    };
}
#endif // DATAGRAM_H_INCLUDED
