//#ifndef UTIL_H_INCLUDED
//#define UTIL_H_INCLUDED
#pragma once
#include <type_traits>
#include <typeinfo>
#include <memory>
#include <cstring>
#include <string>
#include <cstdlib>

#ifndef _MSC_VER
	#include <cxxabi.h>
#endif

#define FMT_HEADER_ONLY
#include "fmt/format.h"

/**
 * \file util.h
 * \brief Header used for utility functions that are universal or very nearly so
 */

/// numerical functions
int closestPowerOfTwo(int i);
int nextBox(int check);

/// string functions
void trim(std::string& str);
void cleancopy(unsigned char* dst, unsigned char* src, int srcLen, int srcOffset = 0);
std::string cleancopySTR(unsigned char* src, int srcLen, int srcOffset = 0);
std::string cleancopySTR(char* src, int srcLen, int srcOffset = 0);
std::string toLowerOut(std::string str);	// print string in lower case, do not alter original
void toLower(std::string& str); 			// alter original to lower case
std::string toUpperOut(std::string str);	// print string in upper case, do not alter original
void toUpper(std::string& str);				// alter original to upper case
bool doesContain(std::string src, std::string searchPattern); // case insensitive substring search

// uses FMT to cleanly create strings
/**
 * Create string object utilizing the FMT library
 */
template <typename... Args>
std::string cleanstring(const char *msg, const Args&... p)
{
    return fmt::format(msg, p...);
}

// source : https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c
// author: Howard Hinnant
// usage: type_name<decltype(myVar)>()
/**
 * Attempt to return the data type of a passed object
 *
 * This was found on StackOverflow, authored by 'Howard Hinnant', url: https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c
 * ConfigReader objects (JSON parser+) are the most common user of this
 * Usage: type_name<decltype(myVar)>()
 */
template <class T>
std::string type_name()
{
	typedef typename std::remove_reference<T>::type TR;
	std::unique_ptr<char, void(*)(void*)> own
	(
		#ifndef _MSC_VER
			abi::__cxa_demangle(typeid(TR).name(), nullptr, nullptr, nullptr),
		#else
			nullptr,
		#endif
		std::free
	);
	std::string r = own != nullptr ? own.get() : typeid(TR).name();
	/*if (std::is_const<TR>::value) { r += " const"; }
	if (std::is_volatile<TR>::value) { r += " volatile"; }
	if (std::is_lvalue_reference<T>::value) { r += "&"; }
	else if (std::is_rvalue_reference<T>::value) { r += "&&"; }*/

	if(r.find("basic_string") != std::string::npos) { r = "string"; }
	else if(r == "unsigned int") { r = "uint32_t"; }
	else if(r == "unsigned long") { r = "uint64_t"; }
	else if(r == "unsigned short") { r = "uint16_t"; }
	else if(r == "int") { r = "int"; }
	else if(r == "long") { r = "int64_t"; }
	else if(r == "short") { r = "int16_t"; }
	else if(r == "glm::tvec2<int, (glm::precision)0>") { r = "glm::ivec2"; }
	else if(r == "glm::tvec3<int, (glm::precision)0>") { r = "glm::ivec3"; }
	else if(r == "glm::tvec4<int, (glm::precision)0>") { r = "glm::ivec4"; }
	else if(r == "glm::tvec2<float, (glm::precision)0>") { r = "glm::vec2"; }
	else if(r == "glm::tvec3<float, (glm::precision)0>") { r = "glm::vec3"; }
	else if(r == "glm::tvec4<float, (glm::precision)0>") { r = "glm::vec4"; }

	return r;
}

std::string getGLErrorString(unsigned int err);

//#endif // UTIL_H_INCLUDED
