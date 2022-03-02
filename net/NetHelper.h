#ifndef NETHELPER_H
#define NETHELPER_H


#include "net/Network.h"
#include "cereal/types/string.hpp"
#include "cereal/types/complex.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/glm.hpp"

#include "cereal/archives/binary.hpp"
#include <sstream>
#include <mutex>

// Ref: https://www.ibm.com/developerworks/community/blogs/5894415f-be62-4bc0-81c5-3956e82276f3/entry/discover_the_traps_when_using_stringstream_str?lang=en

template<class S>
extern void cerealSend(CGameEngine::Network* net, sockaddr_storage* addr, uint16_t opCode, const S& st)
{
    if(!net || !net->isActive()) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "cerealSend()", "No Network object passed or Network not active, skipping."); return; }
    else if(!addr) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "cerealSend()", "No Network address passed, skipping."); return; }

    // lock to ensure stringstream doesn't croak
    std::mutex mut;
    std::unique_lock<std::mutex> ulock(mut);

    // create cereal archive, if possible
    std::stringstream ss(std::ios::out | std::ios::binary);
    try
    {
        {
            cereal::BinaryOutputArchive oarc(ss);
            oarc(st);
        }
    }
    catch(std::exception& e)
    {
       Logger::getInstance().Log(Logs::CRIT, Logs::Utility, "cerealSend", "Failed to read from stringstream! \n\033[1m{}\033[0m\n", e.what());
        return;
    }

    // string conversion to avoid nasty crash
    const std::string& tmp = ss.str();

    if(!tmp.empty())
    {
        // convert to unsigned char array
        uint32_t len = tmp.length();
        unsigned char* ucdata = new unsigned char[len];
        memcpy(ucdata, tmp.c_str(), len);

        // utilize networking class to transmit data over the wire
        // also handles uchar* deletion
        if(!net->isActive()) { Logger::getInstance().Log(Logs::WARN, Logs::Utility, "cerealSend()", "Network connection has closed!"); return; }

        net->send(addr, opCode, &ucdata, len);
    }
    else // empty archive
    {
       Logger::getInstance().Log(Logs::CRIT, Logs::Network, "cerealSend()", "Failure to generate archive to stringstream!");
    }

    // cleanup
    ss.clear();
    std::stringstream().swap(ss);
}

// create cereal archive to repopulate the struct
template<class S>
extern void cerealReceive(unsigned char* data, int dataLength, S& st)
{
    if(!data) { Logger::getInstance().Log(Logs::CRIT, Logs::Utility, "cerealReceive", "Data pointer was null!"); return; }
    else if(dataLength < 3) { Logger::getInstance().Log(Logs::CRIT, Logs::Utility, "cerealReceive", "Data length was too small! (Minimum: 3, Received: {}", dataLength); return; }

    // lock to ensure stringstream doesn't croak
    std::mutex mut;
    std::unique_lock<std::mutex> ulock(mut);

    // create string, then stringstream from data
    std::string str(reinterpret_cast<const char*>(&data[0]), dataLength);
    std::stringstream ss(str, std::ios::in | std::ios::binary);
    try
    {
        {
            cereal::BinaryInputArchive iarc(ss);
            iarc(st);
        }
    }
    catch(std::exception& e)
    {
       Logger::getInstance().Log(Logs::CRIT, Logs::Utility, "cerealReceive", "Failed to read from stringstream! \n\033[1m{}\033[0m\n", e.what());
        return;
    }

    //Logger::getInstance().Log(Logs::DEBUG, "cerealReceive()", "\033[96mReceived data of '{}' size!\033[0m", dataLength);
}

#endif // NETHELPER_H
