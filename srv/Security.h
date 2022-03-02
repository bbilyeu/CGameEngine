#ifndef SECURITY_H_INCLUDED
#define SECURITY_H_INCLUDED
#include <iomanip>
#include <sstream>
#include <string>
#include "openssl/sha.h"

// ref : http://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c

/*class Security
{
    public:
        Security() {}

    private:
};*/

// SHA256 signatures generation
static std::string charSHA256(const char* data, const size_t dataSize)
{
	unsigned char shash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, data, dataSize);
	SHA256_Final(shash, &sha256);
	std::stringstream ss;
	for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		//ss << std::hex << std::setw(2) << std::setfill('0') << (int)shash[i];
		//ss << std::hex << std::setw(2) << std::setfill('0') << (int)shash[i];
		ss << std::setfill('0') << std::setw(sizeof(char)*2) << std::hex << (int)shash[i];
	}
	return ss.str();
}

static std::string strSHA256(const std::string data)
{
	return charSHA256(data.c_str(), data.size());
}

#endif // SECURITY_H_INCLUDED
