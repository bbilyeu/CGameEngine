#ifndef BUILTIN_STRUCTS_INCLUDED
#define BUILTIN_STRUCTS_INCLUDED

#include "common/types.h"

struct ConnectionRequest_Struct
{
    unsigned char title[32];
    SoftwareVersion version = SoftwareVersion(0,0,0);
    uint32_t uniqID = 0;

    template<class Archive>
    void serialize(Archive& arc)
    {
        arc(title, version, uniqID);
    }
};

struct RetryRequest_Struct
{
    RetryRequest_Struct(int seqid, int pktnum, uint64_t nextReqTime) : seqID(seqid), pktNum(pktnum), nextRequestTime(nextReqTime) {}
    uint32_t seqID;
    uint32_t pktNum;
    uint64_t nextRequestTime;

    template<class Archive>
    void serialize(Archive& arc)
    {
        arc(seqID, pktNum, nextRequestTime);
    }
};

/*struct NewConnection_Struct
{
    NetConnection* netCon = nullptr;


};*/

#endif // BUILTIN_STRUCTS_INCLUDED
