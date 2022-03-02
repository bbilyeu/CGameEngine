#ifndef IPCHELPER_H_INCLUDED
#define IPCHELPER_H_INCLUDED

#include "net/NetHelper.h"
#include "net/NetworkIPC.h"

// Ref: https://www.ibm.com/developerworks/community/blogs/5894415f-be62-4bc0-81c5-3956e82276f3/entry/discover_the_traps_when_using_stringstream_str?lang=en

template<class S>
extern void cerealSend(CGameEngine::NetworkIPC* ipc, uint16_t opCode, const uint32_t& senderID, const S& st, uint64_t arrival = 0)
{
    if(!ipc) { Logger::getInstance().Log(Logs::WARN, Logs::Utility, "cerealSend()", "No NetworkIPC object passed, skipping."); return; }

    // lock to ensure stringstream doesn't croak
    std::mutex mut;
    std::unique_lock<std::mutex> ulock(mut);

    // create cereal archive, if possible
    std::stringstream ss(std::ios::out | std::ios::binary);
    {
        cereal::BinaryOutputArchive oarc(ss);
        oarc(st);
    }

    // string conversion to avoid nasty crash
    const std::string& tmp = ss.str();

    if(!tmp.empty())
    {
        // convert to unsigned char array
        uint32_t len = tmp.length();
        unsigned char* ucdata = new unsigned char[len];
        memcpy(ucdata, tmp.c_str(), len);

        ipc->send(opCode, senderID, &ucdata, len, arrival);
    }
    else // empty archive
    {
       Logger::getInstance().Log(Logs::CRIT, Logs::Utility, "cerealSend()", "Failure to generate archive to stringstream!");
    }

    // cleanup
    ss.clear();
    std::stringstream().swap(ss);
}

#endif // IPCHELPER_H_INCLUDED
