#include "common/util.h"

/// numerical functions ///////////////////////////////////////////////////////

/**
 * Find the next highest value (by powers of two)
 */
int closestPowerOfTwo(int i)
{
    i--;
    int pi = 1;
    while (i > 0)
    {
        i >>= 1;
        pi <<= 1;
    }
    return pi;
}


int nextBox(int check)
{
    for(int i = 12; i < 4096; i+=12)
    {
        if(check < i) { return i; }
    }

    // failure
    return 0;
}


/// string functions //////////////////////////////////////////////////////////

/**
 * Trim whitespace, new lines, and other non-text characters from front and end
 * 
 * @param str passed string to trim
 */
void trim(std::string& str)
{
    const char* trimThese = " \t\n\r\f\v";
    str.erase(0, str.find_first_not_of(trimThese)); // left trim
    str.erase(str.find_last_not_of(trimThese) + 1); // right trim
}

/**
 * Cleanly copy between two, existing unsigned char arrays, with optional start offset
 * 
 * @param dst destination unsigned char array (copy to this)
 * @param src source unsigned char array (copy from this)
 * @param srcLen length of the source (src) variable
 * @param srcOffset (optional) offset to start copying from 
 */
void cleancopy(unsigned char* dst, unsigned char* src, int srcLen, int srcOffset /*= 0*/)
{
    if(!src || srcLen == 0) { return; }

    // get size
    int len = 0;
    len = (src[srcLen-1] == 0) ? srcLen : srcLen + 1;

    // create dst, if needed
    if(!dst) { dst = new unsigned char[len]; }

    // actual copy
    memset(dst, 0, len);
    memcpy(dst, src+srcOffset, srcLen);
}

/**
 * Created a new string from a cleanly copied unsigned char array, with optional start offset
 * 
 * @param src source unsigned char array (copy from this)
 * @param srcLen length of the source (src) variable
 * @param srcOffset (optional) offset to start copying from 
 */
std::string cleancopySTR(unsigned char* src, int srcLen, int srcOffset /*= 0*/)
{
    if(!src || srcLen == 0) { return ""; }

    // create string, fill, return
    std::string retVal(srcLen+1, '\0');
    memcpy(&retVal[0], src+srcOffset, srcLen);
    return retVal;
}

/**
 * Created a new string from a cleanly copied char array, with optional start offset
 * 
 * @param src source unsigned char array (copy from this)
 * @param srcLen length of the source (src) variable
 * @param srcOffset (optional) offset to start copying from 
 */
std::string cleancopySTR(char* src, int srcLen, int srcOffset /*= 0*/) //{ return cleancopySTR((unsigned char*)src, srcLen, srcOffset); }
{
    if(!src || srcLen == 0) { return ""; }

    // create string, fill, return
    std::string retVal(srcLen+1, '\0');
    memcpy(&retVal[0], src+srcOffset, srcLen);
    return retVal;
}

/**
 * Create a new string object in all lower case
 * 
 * @param str string passed by value to convert to lower case
 */
std::string toLowerOut(std::string str)
{
	for(char& c : str) { c = std::tolower(c); }
	return str;
}

/**
 * Create a new string object in all upper case
 * 
 * @param str string passed by value to convert to upper case
 */
std::string toUpperOut(std::string str)
{
	for(char& c : str) { c = std::toupper(c); }
	return str;
}

/**
 * Convert an existing string object in all lower case
 * 
 * @param str string passed by reference to convert to lower case
 */
void toLower(std::string& str) { for(char& c : str) { c = std::tolower(c); } }

/**
 * Convert an existing string object in all upper case
 * 
 * @param str string passed by reference to convert to upper case
 */
void toUpper(std::string& str) { for(char& c : str) { c = std::toupper(c); } }

// case insensitive substring search
/**
 * Case insensitive search for a substring
 * 
 * the src string is converted to lower case, along with the searchPattern
 * 
 * @param src string passed by value to search
 * @param searchPattern string passed by value to search against 'src'
 */
bool doesContain(std::string src, std::string searchPattern)
{
	toLower(src);
	toLower(searchPattern);
	return (src.find(searchPattern) != std::string::npos);
}

/**
 * Converted vague OpenGL error unsigned int into a useful error message
 * 
 * @param err unsigned int returned from glGetError()
 */
std::string getGLErrorString(unsigned int err)
{
    switch(err)
    {
		case 0:
			return "GL_NO_ERROR";
		case 1280:	// 0x0500
			return "GL_INVALID_ENUM";
		case 1281: 	// 0x0501
			return "GL_INVALID_VALUE";
		case 1282:	// 0x0502
			return "GL_INVALID_OPERATION";
		case 1283:	// 0x0503
			return "GL_STACK_OVERFLOW";
		case 1284:	// 0x0504:
			return "GL_STACK_UNDERFLOW";
		case 1285:	// 0x0505
			return "GL_OUT_OF_MEMORY";
		case 1286:	// 0x0506
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
		default:
			return "UNKNOWN VALUE '"+std::to_string(err)+"'";
    }
}
