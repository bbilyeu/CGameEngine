#ifndef RAWPACKET_H_INCLUDED
#define RAWPACKET_H_INCLUDED

//#include "cereal/types/common.hpp"
//#include "cereal/archives/binary.hpp"
#include <sstream>
#include <assert.h>
#include <string.h>
#include <algorithm>

/*
    Ref:
        serialization : https://gafferongames.com/post/reading_and_writing_packets/

        http://uscilab.github.io/cereal/serialization_functions.html
        http://uscilab.github.io/cereal/quickstart.html

*/

namespace CGameEngine
{
    class RawPacket
    {
        public:
            unsigned char* buffer = nullptr;
            uint32_t pSize = 0, readPos = 0, writePos = 0;
            uint64_t arrivalTime = 0;

            // read functions
            int8_t readInt8() { assert(readPos + sizeof(int8_t) <= pSize); int8_t value = 0; value = *((int8_t*)(buffer+readPos)); readPos += sizeof(int8_t); return value; }
            int16_t readInt16() { assert(readPos + sizeof(int16_t) <= pSize); int16_t value = 0; value = *((int16_t*)(buffer+readPos)); readPos += sizeof(int16_t); return value; }
            int32_t readInt32() { assert(readPos + sizeof(int32_t) <= pSize); int32_t value = 0; value = *((int32_t*)(buffer+readPos)); readPos += sizeof(int32_t); return value; }
            uint8_t readUInt8() { assert(readPos + sizeof(uint8_t) <= pSize); uint8_t value = 0; value = *((uint8_t*)(buffer+readPos)); readPos += sizeof(uint8_t); return value; }
            uint16_t readUInt16() { assert(readPos + sizeof(uint16_t) <= pSize); uint16_t value = 0; value = *((uint16_t*)(buffer+readPos)); readPos += sizeof(uint16_t); return value; }
            uint32_t readUInt32() { assert(readPos + sizeof(uint32_t) <= pSize); uint32_t value = 0; value = *((uint32_t*)(buffer+readPos)); readPos += sizeof(uint32_t); return value; }
            uint64_t readUInt64() { assert(readPos + sizeof(uint64_t) <= pSize); uint64_t value = 0; value = *((uint64_t*)(buffer+readPos)); readPos += sizeof(uint64_t); return value; }
            unsigned char readUChar() { assert(readPos + sizeof(unsigned char) <= pSize); unsigned char value = '\0'; value = *((unsigned char*)(buffer+readPos)); readPos += sizeof(unsigned char); return value; }
            void readUCharArr(unsigned char* ptr, uint32_t len) { assert(readPos + len <= pSize); memcpy(ptr, buffer+readPos, len); readPos += len; }

            // write functions
            void writeInt8(int8_t val) { assert(writePos + sizeof(int8_t) <= pSize); *((int8_t*)(buffer+writePos)) = val; writePos += sizeof(int8_t); }
            void writeInt16(int16_t val) { assert(writePos + sizeof(int16_t) <= pSize); *((int16_t*)(buffer+writePos)) = val; writePos += sizeof(int16_t); }
            void writeInt32(int32_t val) { assert(writePos + sizeof(int32_t) <= pSize); *((int32_t*)(buffer+writePos)) = val; writePos += sizeof(int32_t); }
            void writeUInt8(uint8_t val) { assert(writePos + sizeof(uint8_t) <= pSize); *((uint8_t*)(buffer+writePos)) = val; writePos += sizeof(uint8_t); }
            void writeUInt16(uint16_t val) { assert(writePos + sizeof(uint16_t) <= pSize); *((uint16_t*)(buffer+writePos)) = val; writePos += sizeof(uint16_t); }
            void writeUInt32(uint32_t val) { assert(writePos + sizeof(uint32_t) <= pSize); *((uint32_t*)(buffer+writePos)) = val; writePos += sizeof(uint32_t); }
            void writeUInt64(uint64_t val) { assert(writePos + sizeof(uint64_t) <= pSize); *((uint64_t*)(buffer+writePos)) = val; writePos += sizeof(uint64_t); }
            //void writeUChar(const unsigned char c) { assert(writePos + sizeof(unsigned char) <= pSize); memcpy(buffer+writePos, c, sizeof(unsigned char)); writePos += sizeof(unsigned char); }
            void writeUChar(const unsigned char val) { assert(writePos + sizeof(unsigned char) <= pSize); *((unsigned char*)(buffer+writePos)) = val; writePos += sizeof(unsigned char); }
            void writeUCharArr(const unsigned char* ptr, uint32_t len) { assert(writePos + len <= pSize); memcpy(buffer+writePos, ptr, len); writePos += len; }

            RawPacket(uint32_t len) : buffer(new unsigned char[len]), pSize(len) { memset(buffer, 0, pSize); } // initialization ctor
            RawPacket(const RawPacket& p) : RawPacket(p.pSize) { copy(*this, p); } // copy ctor
            RawPacket(RawPacket&& p) noexcept : RawPacket(p.pSize)  { swap(*this, p); } // move ctor
            RawPacket& operator=(const RawPacket& p) { copy(*this, p); return *this; } // copy assignment
            RawPacket& operator=(RawPacket&& p) noexcept { swap(*this, p); return *this; } // move assignment

            friend void copy(RawPacket& dst, const RawPacket& src)
            {
                if(&dst != &src)
                {
                    memcpy(dst.buffer, src.buffer, src.pSize);
                    dst.pSize = src.pSize;
                    dst.readPos = src.readPos;
                    dst.writePos = src.writePos;
                    dst.arrivalTime = src.arrivalTime;
                }
            }

            friend void swap(RawPacket& dst, RawPacket& src)
            {
                if(&dst != &src)
                {
                    using std::swap;
                    if(dst.buffer) { delete[] dst.buffer; dst.buffer = nullptr; }
                    swap(dst.buffer, src.buffer);
                    swap(dst.pSize, src.pSize);
                    swap(dst.readPos, src.readPos);
                    swap(dst.writePos, src.writePos);
                    swap(dst.arrivalTime, src.arrivalTime);
                }
            }

        protected:
            RawPacket(const unsigned char* buf = nullptr, uint32_t len = 0, const uint64_t& arrival = 0) : pSize(len), arrivalTime(arrival)
            {
                // build buffer, if possible
                if(pSize)
                {
                    buffer = new unsigned char[pSize];
                    memset(buffer, 0, pSize);

                    // copy passed data
                    if(buf) { memcpy(buffer, buf, pSize); }
                }
            }

            virtual ~RawPacket()
            {
                if(buffer) { delete[] buffer; buffer = nullptr; }
            }

            /// Remove excess whitespace or null characters
            void trimBuffer()
            {
                unsigned char* tmp = new unsigned char[writePos];
                memset(tmp, 0, writePos);
                memcpy(tmp, buffer, writePos);
                safeDeleteArray(buffer);
                std::swap(tmp, buffer);
                safeDeleteArray(tmp);
                pSize = writePos;
            }
    };
}

#endif // RAWPACKET_H_INCLUDED
