#include "net/net_util.h"
#include <sstream>
#include <iomanip>

//#include <sstream>
#include <stdio.h>
#include <iostream>
//#include <stddef.h>
//#include <stdint.h>

bool isSameSource(sockaddr_storage* sas, addrinfo* addr)
{
    if(!sas || !addr) { return false; } // quick exit

    sockaddr_storage* saddr = (struct sockaddr_storage*)addr->ai_addr;

    // are both IPv4 or IPv6
    if(sas->ss_family != saddr->ss_family) { return false; }
    else
    {
        if(sas->ss_family == AF_INET)
        {
           sockaddr_in* sai = (struct sockaddr_in*)sas;
           sockaddr_in* saddri = (struct sockaddr_in*)saddr;

           if(memcmp(&sai->sin_addr, &saddri->sin_addr, sizeof(in_addr)) != 0/* || memcmp(&sai->sin_port, &saddri->sin_port, sizeof(in_port_t)) != 0*/) { return false; }
        }
        else
        {
            sockaddr_in6* sai = (struct sockaddr_in6*)sas;
            sockaddr_in6* saddri = (struct sockaddr_in6*)saddr;

            if(memcmp(&sai->sin6_addr, &saddri->sin6_addr, sizeof(in6_addr)) != 0 /*|| memcmp(&sai->sin6_port, &saddri->sin6_port, sizeof(in_port_t)) != 0*/) { return false; }
        }
    }

    return true;
}

uint16_t getPort(const sockaddr_storage* ss)
{
    // catch for failure
    if(!ss) { return 0; }

    uint16_t retVal = 0;

    switch(ss->ss_family)
    {
        case AF_INET:
        {
            struct sockaddr_in* sai = (struct sockaddr_in*)ss;
            retVal = ntohs(sai->sin_port);
            break;
        }
        case AF_INET6:
        {
            struct sockaddr_in6* sai6 = (struct sockaddr_in6*)ss;
            retVal = ntohs(sai6->sin6_port);
            break;
        }
    }

    return retVal;
}

uint16_t getPort(const addrinfo* adi)
{
    if(!adi) { return 0; }
    else { return getPort((struct sockaddr_storage*)adi->ai_addr); }
}

uint16_t getPort(const std::string& IPString)
{
    std::string ignored = "";
    uint16_t retVal = 0;
    splitIP(IPString, ignored, retVal);
    return retVal;
}

std::string dumpPacket(const unsigned char* buffer, uint32_t length)
{
    // use 16 columns as hex has 16 possible digits
	std::ostringstream out;
	out << "\n";
	if (length == 0 || length > 39565) { return ""; }
	int columns = 16;
	int skipping = 0;

	char output[4];
	int j = 0;
	auto textOutput = new char[columns + 1];
	memset(textOutput, 0, columns + 1);
	uint32_t i;
	for (i = skipping; i < length; i++)
	{
		if ((i - skipping) % columns == 0)
		{
			if (i != skipping)
			{
				out << " | " << textOutput << std::endl;
				out << std::setw(4) << std::setfill(' ') << i - skipping << ": ";
            }
			memset(textOutput, 0, columns + 1);
			j = 0;
		}
		else if ((i - skipping) % (columns / 2) == 0) { out << "- "; }
		sprintf(output, "%02X ", (unsigned char)buffer[i]);
		out << output;

		if (buffer[i] >= 32 && buffer[i] < 127) { textOutput[j++] = buffer[i]; }
		else { textOutput[j++] = '.'; }
	}
	uint32_t k = ((i - skipping) - 1) % columns;
	if (k < 8) { out << "  "; }
	for (uint32_t h = k + 1; h < columns; h++) { out << "   "; }
	out << " | " << textOutput << std::endl;
	safeDeleteArray(textOutput);

	return out.str();
}

std::string getIPString(const sockaddr_storage* ss)
{
    // catch for failure
    if(!ss) { return "NULL"; }

    std::string str;
    switch(ss->ss_family)
    {
        case AF_INET:
        {
            struct sockaddr_in* sai = (struct sockaddr_in*)ss;
            str.resize(INET_ADDRSTRLEN+1, '\0');
            inet_ntop(AF_INET, &sai->sin_addr, &str[0], INET_ADDRSTRLEN);
            str += ":" + std::to_string(ntohs(sai->sin_port));
            break;
        }
        case AF_INET6:
        {
            struct sockaddr_in6* sai6 = (struct sockaddr_in6*)ss;
            str.resize(INET6_ADDRSTRLEN+1, '\0');
            inet_ntop(AF_INET, &sai6->sin6_addr, &str[0], INET6_ADDRSTRLEN);
            str += ":" + std::to_string(ntohs(sai6->sin6_port));
            break;
        }
        default:
            return "NULL";
    }

    return str;
}

std::string getIPString(const addrinfo* adi)
{
    if(!adi) { return "NULL"; }
    else { return getIPString((struct sockaddr_storage*)adi->ai_addr); }
}

void splitIP(const std::string& IPString, std::string& ipDst, uint16_t& portDst)
{
    int delimiterPOS = IPString.find(':', 0);
    if(delimiterPOS <= 7) // failure
    {
        ipDst = "NULL";
        portDst = 0;
        return;
    }

    const char* trimThese = " \t\n\r\f\v";
    ipDst = IPString.substr(0, delimiterPOS);
    std::cout<<"\033[1mipDst ["<<ipDst<<"], delimiterPOS ["<<delimiterPOS<<"]\033[0m\n";
    ipDst.erase(0, ipDst.find_first_not_of(trimThese)); // left trim
    std::cout<<"\033[1mipDst ["<<ipDst<<"], delimiterPOS ["<<delimiterPOS<<"]\033[0m\n";
    ipDst.erase(ipDst.find_last_not_of(trimThese) + 1); // right trim
    std::cout<<"\033[1mipDst ["<<ipDst<<"], delimiterPOS ["<<delimiterPOS<<"]\033[0m\n";

    std::string port = IPString.substr(delimiterPOS+1, IPString.length() - delimiterPOS - 1);
    try { int p = std::stoi(port); portDst = static_cast<uint16_t>(p); }
    catch (const std::exception& e) { ipDst = "NULL", portDst = 0; }
}

/*std::string hexify(uint32_t data)
{
    std::stringstream sstream;
    int slots = data > 0 ? (int) log10 ((double) data) + 1 : 1; // get digits
    sstream << std::setfill('0') << std::setw(sizeof(char)*slots) << std::hex << data;
    return sstream.str();
}

unsigned int hexToInt(std::string data)
{
    return std::stoul(data, nullptr, 16);
}

void hexify(unsigned char* dest, std::string val, int slots)
{
    std::stringstream sstream;
    sstream << std::setw(sizeof(char)*slots) << std::hex << val;
    const char* retVal = sstream.str().c_str();
    dest = new unsigned char[slots];
    memset(&dest, 0, slots);
    memcpy(&dest, &retVal, slots);
    retVal = nullptr;
}*/
