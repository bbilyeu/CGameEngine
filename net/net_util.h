#ifndef NET_UTIL_H
#define NET_UTIL_H

#include "common/types.h"
#include <arpa/inet.h> // inet_ntoa
#include <netdb.h>
#include <string>
#include <unistd.h> // close()
#include "net/Builtin_Structs.h"
//#include <netinet/in.h>

/// handle proper library loading
#if PLATFORM == PLATFORM_WINDOWS
    #include <winsock2.h>
    #pragma comment( lib, "wsock32.lib" )
#elif PLATFORM == PLATFORM_MAC ||  PLATFORM == PLATFORM_LINUX
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <fcntl.h>
#endif

bool isSameSource(sockaddr_storage* sas, addrinfo* addr);
uint16_t getPort(const sockaddr_storage* ss);
uint16_t getPort(const addrinfo* ad);
uint16_t getPort(const std::string& IPString, uint16_t& port);
std::string dumpPacket(const unsigned char* buffer, uint32_t length);
std::string getIPString(const sockaddr_storage* ss);
std::string getIPString(const addrinfo* adi);
void splitIP(const std::string& IPString, std::string& ipDst, uint16_t& portDst);

//std::string hexify(uint32_t data);
//unsigned int hexToInt(std::string data);
//void hexify(char* data, int slots);

#endif // NET_UTIL_H
