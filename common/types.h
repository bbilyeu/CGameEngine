#pragma once
#define GL_GLEXT_PROTOTYPES
//#include <GL/glcorearb.h>
#include <GL/glew.h>

#include <glm/glm.hpp>
#include "srv/Logger.h"
#include <stdint.h>
#include <regex>

/*
    Ref:
        https://stackoverflow.com/questions/1898153/how-to-determine-if-memory-is-aligned-testing-for-alignment-not-aligning
            Alignment macro
*/

/**
 * @file types.h
 * @brief Header created to handle/convert types to c++ standard, as well as common structs
 */

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_LINUX    3

/// handle platform discovery
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #define PLATFORM PLATFORM_MAC
#else
    #define PLATFORM PLATFORM_LINUX
#endif

#ifndef __cplusplus
    typedef enum { true, false } bool;
#endif

#ifdef _WINDOWS
	#pragma warning( disable : 4200 )
#endif

#ifndef _WIN32
	// converting Windows C++ to standard C++
	/// \TODO: Determine if this even matters or can we just stick to c++ standards.
	typedef unsigned long DWORD;
	typedef unsigned char BYTE;
	typedef char CHAR;
	typedef unsigned short WORD;
	typedef float FLOAT;
	typedef FLOAT *PFLOAT;
	typedef BYTE *PBYTE,*LPBYTE;
	typedef int *PINT,*LPINT;
	typedef WORD *PWORD,*LPWORD;
	typedef long *LPLONG, LONG;
	typedef DWORD *PDWORD,*LPDWORD;
	typedef int INT;
	typedef unsigned int UINT,*PUINT,*LPUINT;
#endif

/**
 * Attempt to delete a raw pointer after 'safely' checking existence
 */
template <typename T>
void safeDelete(T*& ptr) { if(ptr && ptr != nullptr) { delete ptr; ptr = nullptr; } }

/**
 * Attempt to delete a raw array pointer after 'safely' checking existence
 */
template <typename T>
void safeDeleteArray(T*& ptr) { if(ptr && ptr != nullptr) { delete[] ptr; ptr = nullptr; } }

/**
 * struct for holding major, minor, and patch software versions (MAY GET SCRAPPED)
 *
 * ref: http://semver.org/
 */
struct SoftwareVersion
{
    SoftwareVersion(uint8_t M, uint8_t m, uint8_t p) : sw_major(M), sw_minor(m), sw_patch(p) {}
    ~SoftwareVersion() = default;
    uint8_t sw_major = 0;
    uint8_t sw_minor = 0;
    uint8_t sw_patch = 0;
};

/*template< class CharT, class Traits, class Alloc >
bool std::string::operator~(const std::basic_string<CharT,Traits,Alloc>& lhs, const std::basic_string<CharT,Traits,Alloc>& rhs)
{
	 return std::regex_search(lhs, rhs);
}*/

/**
 * text justification
 */
namespace Justification
{
	enum ALIGNMENT
	{
		NONE = 0, /**< effectively null */
		Left, /**< left aligned */
		Center, /**< center aligned */
		Right, /**< right aligned */
		END /**< end of usable values*/
	};
}

/**
 * Handles the OpenGL 'TextureAddress' format used in bindless operations
 */
struct TextureAddress
{
	uint64_t containerHandle; /** openGL provided array ID */
	int sliceNumber; /** openGL provided array layer ID */
	//int reserved;

	const inline bool defined() const { return (bool)(containerHandle > 0); }
	const inline bool exists() const { return (bool)(containerHandle > 0); }
	inline bool operator==(const TextureAddress& other) { return (other.containerHandle == this->containerHandle && other.sliceNumber == this->sliceNumber); }
	inline bool operator!=(const TextureAddress& other) { return (other.containerHandle != this->containerHandle || other.sliceNumber != this->sliceNumber); }
	inline bool operator>(const TextureAddress& other) { return (this->containerHandle > other.containerHandle || (this->containerHandle == other.containerHandle && this->sliceNumber > other.sliceNumber)); }
	inline bool operator<(const TextureAddress& other) { return (this->containerHandle < other.containerHandle || (this->containerHandle == other.containerHandle && this->sliceNumber < other.sliceNumber)); }
};

/**
 * Handles 'Vertex' objects, consisting of x,y,z position and x,y UV-coords
 */
struct Vertex
{
	glm::vec3 position; /** x,y,z coords, 2d uses Z as 'depth' (back<-->front) */
	glm::vec2 uv; /* x,y UV-coords */
	inline bool operator==(const Vertex& other) { return (other.position == this->position && other.uv == this->uv); }
	inline bool operator!=(const Vertex& other) { return (other.position != this->position || other.uv != this->uv); }
	inline bool operator>(const Vertex& other) { return (this->position.x > other.position.x && this->position.y > other.position.y
													&& this->position.z > other.position.z && this->uv.x > other.uv.x
													&& this->uv.y > other.uv.y); }
	inline bool operator<(const Vertex& other) { return !(*this > other); }
};

/**
 * Handles a 'debug' version of the 'Vertex' object, which is mainly position and fill color
 */
struct DebugVertex
{
	glm::vec3 position = glm::vec3(0.0f); /** x,y,z position, defaults to 0,0,0 */
	glm::ivec4 color = glm::ivec4(255); /** color used for fill/lines, default is white (255,255,255, 255) */
	inline bool operator==(const DebugVertex& other) { return (other.position == this->position && other.color == this->color); }
	inline bool operator!=(const DebugVertex& other) { return (other.position != this->position || other.color != this->color); }
};

// common colors
static glm::ivec4 COLOR_NONE(0, 0, 0, 0);
static glm::ivec4 COLOR_BLACK(0, 0, 0, 255);
static glm::ivec4 COLOR_WHITE(255, 255, 255, 255);
static glm::ivec4 COLOR_GREY(163, 171, 183, 255);
static glm::ivec4 COLOR_RED(255, 0, 0, 255);
static glm::ivec4 COLOR_GREEN(0, 255, 0, 255);
static glm::ivec4 COLOR_BLUE(0, 0, 255, 255);
static glm::ivec4 COLOR_YELLOW(255, 255, 0, 255);
static glm::ivec4 COLOR_PURPLE(128, 0, 187, 255);
static glm::ivec4 COLOR_ORANGE(255, 128, 0, 255);
static glm::ivec4 COLOR_TRANSPARENT(255, 255, 255, 0);
