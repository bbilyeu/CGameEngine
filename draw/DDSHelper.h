#pragma once

#include "common/types.h"
#include <cstdlib>
#include <stdio.h>
#include <memory.h>

/**
 * @file DDSHelper.h
 * @brief utilities to assist in importing textures (credits to nVidia)
 */

#ifndef MAKEFOURCC
#define MAKEFOURCC(a, b, c, d) \
                  (((d & 0xFF) << 24) \
                |  ((c & 0xFF) << 16) \
                |  ((b & 0xFF) <<  8) \
                |  ((a & 0xFF) <<  0))
#endif

/**
 * @brief details specific to DDS files
 */
struct TextureDetails
{
    uint32_t dWidth;        /**< width in pixels */
    uint32_t dHeight;       /**< height in pixels */

    GLsizei mipmapLevels;   /**< number of mipmap level */
    void* imageData;        /**< raw image data */
    GLsizei* mipmapSizes;   /**< pointer each mipmap's size */
    GLenum glFormat;        /**< specific format type */

    bool bCompressed;       /**< compressed or not */

    TextureDetails() { memset(this, 0, sizeof(*this)); }
    ~TextureDetails()
    {
        if(imageData) { free(imageData); }
        if(mipmapSizes) { free(mipmapSizes); }
    }

    GLint MipMapHeight(GLint mipMap) { return std::max(1, (int)(dHeight >> mipMap)); }
    GLint MipMapWidth(GLint mipMap) { return std::max(1, (int)(dWidth >> mipMap)); }
};

// forward decl
bool readDDSFromFile(std::string fileName, TextureDetails* retVal);


struct DDS_PIXELFORMAT
{
    uint32_t dSize;
    uint32_t dFlags;
    uint32_t dFourCC;
    uint32_t dRGBBitCount;
    uint32_t dRBitMask;
    uint32_t dGBitMask;
    uint32_t dBBitMask;
    uint32_t dABitMask;
};

struct DDS_HEADER
{
    uint32_t           dSize;
    uint32_t           dFlags;
    uint32_t           dHeight;
    uint32_t           dWidth;
    uint32_t           dPitchOrLinearSize;
    uint32_t           dDepth;
    uint32_t           dMipMapCount;
    uint32_t           dReserved1[11];
    DDS_PIXELFORMAT		ddspf;
    uint32_t           dCaps;
    uint32_t           dCaps2;
    uint32_t           dCaps3;
    uint32_t           dCaps4;
    uint32_t           dReserved2;

    DDS_HEADER()
    {
        memset(this, 0, sizeof(*this));
    }
};

#ifndef DXGI_FORMAT_DEFINED
enum DXGI_FORMAT
{
    DXGI_FORMAT_UNKNOWN                     = 0,
    DXGI_FORMAT_R32G32B32A32_TYPELESS       = 1,
    DXGI_FORMAT_R32G32B32A32_FLOAT          = 2,
    DXGI_FORMAT_R32G32B32A32_UINT           = 3,
    DXGI_FORMAT_R32G32B32A32_SINT           = 4,
    DXGI_FORMAT_R32G32B32_TYPELESS          = 5,
    DXGI_FORMAT_R32G32B32_FLOAT             = 6,
    DXGI_FORMAT_R32G32B32_UINT              = 7,
    DXGI_FORMAT_R32G32B32_SINT              = 8,
    DXGI_FORMAT_R16G16B16A16_TYPELESS       = 9,
    DXGI_FORMAT_R16G16B16A16_FLOAT          = 10,
    DXGI_FORMAT_R16G16B16A16_UNORM          = 11,
    DXGI_FORMAT_R16G16B16A16_UINT           = 12,
    DXGI_FORMAT_R16G16B16A16_SNORM          = 13,
    DXGI_FORMAT_R16G16B16A16_SINT           = 14,
    DXGI_FORMAT_R32G32_TYPELESS             = 15,
    DXGI_FORMAT_R32G32_FLOAT                = 16,
    DXGI_FORMAT_R32G32_UINT                 = 17,
    DXGI_FORMAT_R32G32_SINT                 = 18,
    DXGI_FORMAT_R32G8X24_TYPELESS           = 19,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
    DXGI_FORMAT_R10G10B10A2_TYPELESS        = 23,
    DXGI_FORMAT_R10G10B10A2_UNORM           = 24,
    DXGI_FORMAT_R10G10B10A2_UINT            = 25,
    DXGI_FORMAT_R11G11B10_FLOAT             = 26,
    DXGI_FORMAT_R8G8B8A8_TYPELESS           = 27,
    DXGI_FORMAT_R8G8B8A8_UNORM              = 28,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
    DXGI_FORMAT_R8G8B8A8_UINT               = 30,
    DXGI_FORMAT_R8G8B8A8_SNORM              = 31,
    DXGI_FORMAT_R8G8B8A8_SINT               = 32,
    DXGI_FORMAT_R16G16_TYPELESS             = 33,
    DXGI_FORMAT_R16G16_FLOAT                = 34,
    DXGI_FORMAT_R16G16_UNORM                = 35,
    DXGI_FORMAT_R16G16_UINT                 = 36,
    DXGI_FORMAT_R16G16_SNORM                = 37,
    DXGI_FORMAT_R16G16_SINT                 = 38,
    DXGI_FORMAT_R32_TYPELESS                = 39,
    DXGI_FORMAT_D32_FLOAT                   = 40,
    DXGI_FORMAT_R32_FLOAT                   = 41,
    DXGI_FORMAT_R32_UINT                    = 42,
    DXGI_FORMAT_R32_SINT                    = 43,
    DXGI_FORMAT_R24G8_TYPELESS              = 44,
    DXGI_FORMAT_D24_UNORM_S8_UINT           = 45,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT        = 47,
    DXGI_FORMAT_R8G8_TYPELESS               = 48,
    DXGI_FORMAT_R8G8_UNORM                  = 49,
    DXGI_FORMAT_R8G8_UINT                   = 50,
    DXGI_FORMAT_R8G8_SNORM                  = 51,
    DXGI_FORMAT_R8G8_SINT                   = 52,
    DXGI_FORMAT_R16_TYPELESS                = 53,
    DXGI_FORMAT_R16_FLOAT                   = 54,
    DXGI_FORMAT_D16_UNORM                   = 55,
    DXGI_FORMAT_R16_UNORM                   = 56,
    DXGI_FORMAT_R16_UINT                    = 57,
    DXGI_FORMAT_R16_SNORM                   = 58,
    DXGI_FORMAT_R16_SINT                    = 59,
    DXGI_FORMAT_R8_TYPELESS                 = 60,
    DXGI_FORMAT_R8_UNORM                    = 61,
    DXGI_FORMAT_R8_UINT                     = 62,
    DXGI_FORMAT_R8_SNORM                    = 63,
    DXGI_FORMAT_R8_SINT                     = 64,
    DXGI_FORMAT_A8_UNORM                    = 65,
    DXGI_FORMAT_R1_UNORM                    = 66,
    DXGI_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
    DXGI_FORMAT_R8G8_B8G8_UNORM             = 68,
    DXGI_FORMAT_G8R8_G8B8_UNORM             = 69,
    DXGI_FORMAT_BC1_TYPELESS                = 70,
    DXGI_FORMAT_BC1_UNORM                   = 71,
    DXGI_FORMAT_BC1_UNORM_SRGB              = 72,
    DXGI_FORMAT_BC2_TYPELESS                = 73,
    DXGI_FORMAT_BC2_UNORM                   = 74,
    DXGI_FORMAT_BC2_UNORM_SRGB              = 75,
    DXGI_FORMAT_BC3_TYPELESS                = 76,
    DXGI_FORMAT_BC3_UNORM                   = 77,
    DXGI_FORMAT_BC3_UNORM_SRGB              = 78,
    DXGI_FORMAT_BC4_TYPELESS                = 79,
    DXGI_FORMAT_BC4_UNORM                   = 80,
    DXGI_FORMAT_BC4_SNORM                   = 81,
    DXGI_FORMAT_BC5_TYPELESS                = 82,
    DXGI_FORMAT_BC5_UNORM                   = 83,
    DXGI_FORMAT_BC5_SNORM                   = 84,
    DXGI_FORMAT_B5G6R5_UNORM                = 85,
    DXGI_FORMAT_B5G5R5A1_UNORM              = 86,
    DXGI_FORMAT_B8G8R8A8_UNORM              = 87,
    DXGI_FORMAT_B8G8R8X8_UNORM              = 88,
    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
    DXGI_FORMAT_B8G8R8A8_TYPELESS           = 90,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
    DXGI_FORMAT_B8G8R8X8_TYPELESS           = 92,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
    DXGI_FORMAT_BC6H_TYPELESS               = 94,
    DXGI_FORMAT_BC6H_UF16                   = 95,
    DXGI_FORMAT_BC6H_SF16                   = 96,
    DXGI_FORMAT_BC7_TYPELESS                = 97,
    DXGI_FORMAT_BC7_UNORM                   = 98,
    DXGI_FORMAT_BC7_UNORM_SRGB              = 99,
    DXGI_FORMAT_AYUV                        = 100,
    DXGI_FORMAT_Y410                        = 101,
    DXGI_FORMAT_Y416                        = 102,
    DXGI_FORMAT_NV12                        = 103,
    DXGI_FORMAT_P010                        = 104,
    DXGI_FORMAT_P016                        = 105,
    DXGI_FORMAT_420_OPAQUE                  = 106,
    DXGI_FORMAT_YUY2                        = 107,
    DXGI_FORMAT_Y210                        = 108,
    DXGI_FORMAT_Y216                        = 109,
    DXGI_FORMAT_NV11                        = 110,
    DXGI_FORMAT_AI44                        = 111,
    DXGI_FORMAT_IA44                        = 112,
    DXGI_FORMAT_P8                          = 113,
    DXGI_FORMAT_A8P8                        = 114,
    DXGI_FORMAT_B4G4R4A4_UNORM              = 115,
    DXGI_FORMAT_FORCE_UINT                  = 0xffffffffUL
};
#endif

const uint32_t DDPF_ALPHAPIXELS	= 0x1;
const uint32_t DDPF_ALPHA		= 0x2;
const uint32_t DDPF_FOURCC		= 0x4;
const uint32_t DDPF_RGB			= 0x40;
const uint32_t DDPF_YUV	        = 0x200;
const uint32_t DDPF_LUMINANCE	= 0x20000;

#if !defined(D3D10_APPEND_ALIGNED_ELEMENT)
enum D3D10_RESOURCE_DIMENSION
{
  D3D10_RESOURCE_DIMENSION_UNKNOWN    = 0,
  D3D10_RESOURCE_DIMENSION_BUFFER     = 1,
  D3D10_RESOURCE_DIMENSION_TEXTURE1D  = 2,
  D3D10_RESOURCE_DIMENSION_TEXTURE2D  = 3,
  D3D10_RESOURCE_DIMENSION_TEXTURE3D  = 4
};
#endif

struct DDS_HEADER_DXT10
{
  DXGI_FORMAT              dxgiFormat;
  D3D10_RESOURCE_DIMENSION resourceDimension;
  UINT                     miscFlag;
  UINT                     arraySize;
  UINT                     reserved;
};

struct DDS_FILEHEADER
{
    uint32_t   magicNumber;
    DDS_HEADER      header;

    DDS_FILEHEADER() { memset(this, 0, sizeof(*this)); }
};

inline bool readDDSFromFile_DXT1(const DDS_HEADER& header, FILE* file, TextureDetails* retVal);

const uint32_t kDDS_MagicNumber = MAKEFOURCC('D', 'D', 'S', ' ');
const uint32_t kDDS_DX10 = MAKEFOURCC('D', 'X', '1', '0');

// ------------------------------------------------------------------------------------------------
inline bool readDDSFromFile(std::string fileName, TextureDetails* retVal)
{
    FILE* ddsFile = std::fopen(fileName.c_str(), "r");
    if(!ddsFile) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "DDSHelper,readDDSFromFile()", "Could not open file [{}]", fileName); } // 'exit'

    bool success = false;
    DDS_FILEHEADER fileHeader;

    // read in file header info
    if(std::fread(&fileHeader, sizeof(fileHeader), 1, ddsFile) != 1)
    {
		Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "DDSHelper,readDDSFromFile()", "fread returned a non-zero exit code!");
	}

    // Sanity check magic values.
    if (fileHeader.magicNumber != kDDS_MagicNumber || fileHeader.header.dSize != 124 || fileHeader.header.ddspf.dSize != 32)
    {
		Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "DDSHelper,readDDSFromFile()",
			"Alignment Error: fileHeader.magicNumber [{}] vs kDDS_MagicNumber [{}], header.dSize [{}] vs [124], header.ddspf.dSize [{}] vs [32]",
			fileHeader.magicNumber, kDDS_MagicNumber, fileHeader.header.dSize, fileHeader.header.ddspf.dSize);
	}

    if((fileHeader.header.ddspf.dFlags & DDPF_FOURCC) != 0)
    {
        if(fileHeader.header.ddspf.dFourCC == MAKEFOURCC('D', 'X', 'T', '1')) { success = readDDSFromFile_DXT1(fileHeader.header, ddsFile, retVal); }
        else
		{
			Logger::getInstance().Log(Logs::FATAL, Logs::Drawing,
				"DDSHelper,readDDSFromFile()", "fileHeader.header.ddspf.dFourCC [{}] !=  MAKEFOURCC('D', 'X', 'T', '1') [{}]",
				fileHeader.header.ddspf.dFourCC, MAKEFOURCC('D', 'X', 'T', '1'));
		}
    }
    else
    {
		Logger::getInstance().Log(Logs::FATAL, Logs::Drawing,
			"DDSHelper,readDDSFromFile()", "fileHeader.header.ddspf.dFlags [{}] & DDPF_FOURCC [{}] == [{}] (expected anything non-zero)",
			fileHeader.header.ddspf.dFlags, DDPF_FOURCC, (fileHeader.header.ddspf.dFlags & DDPF_FOURCC));
	}

    if(success)
    {
        retVal->dHeight = fileHeader.header.dHeight;
        retVal->dWidth = fileHeader.header.dWidth;
    }
    // else case not needed as readDDSFromFile_DXT1 can abort, if needed

    // cleanup
    fclose(ddsFile);
    return success;
}

// ------------------------------------------------------------------------------------------------
inline bool readDDSFromFile_DXT1(const DDS_HEADER& header, FILE* file, TextureDetails* retVal)
{
    const uint32_t kBlockSizeBytes = 8;
    const uint32_t kColsPerBlock = 4;
    const uint32_t kRowsPerBlock = 4;

    size_t totalSize = 0;
    uint32_t mipmaps = std::max(1, (int)header.dMipMapCount);

    retVal->mipmapLevels = mipmaps;
    retVal->mipmapSizes = (GLsizei*)malloc(mipmaps * sizeof(GLsizei));

    assert(sizeof(unsigned char) == 1);

    uint32_t mipWidth = header.dWidth;
    uint32_t mipHeight = header.dHeight;

    for(uint32_t mip = 0; mip < mipmaps; ++mip)
    {
        uint32_t pitchBlocks = (mipWidth + (kColsPerBlock - 1)) / kColsPerBlock;
        uint32_t heightBlocks = (mipHeight + (kRowsPerBlock - 1)) / kRowsPerBlock;
        GLsizei mipmapSize = pitchBlocks * heightBlocks * kBlockSizeBytes;
        retVal->mipmapSizes[mip] = mipmapSize;

        totalSize += mipmapSize;
        mipWidth = std::max(1, (int)(mipWidth >> 1));
        mipHeight = std::max(1, (int)(mipHeight >> 1));
    }

    unsigned char* buffer = (unsigned char*)malloc(totalSize);
    //size_t bytesRead = fread(buffer, totalSize, 1, file);
    //int freadRet = fread(buffer, totalSize, 1, file);
    if(fread(buffer, totalSize, 1, file) != 1)
    {
		Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "DDSHelper,readDDSFromFile_DXT1()", "Failed to read file of totalSize [{}]", totalSize);
        free(buffer);
        buffer = 0;
        return false;
    }

    retVal->imageData = buffer;
    retVal->bCompressed = true;
    retVal->glFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;

    return true;
}


// Commit: 0517634

//void Map::readMap(const std::string& data, b2World* world, std::vector<Terrain*>& terr, std::vector<Destructible*>& dstr, std::vector<glm::vec2>& spawnpoints, bool isRawData /*= false*/)
//{
//    // fread : http://www.cplusplus.com/reference/cstdio/fread/
//
//    std::vector<std::string> lines;
//    std::string buffer = ""; // temporary string
//    size_t pos = 0;
//    size_t lSize = 0;
//    FILE* lvlFile;
//    const char* trimThese = " \t\n\r\f\v";
//
//    if(!isRawData)
//    {
//        // open file
//        lvlFile = fopen(data.c_str(), "rb"); // wb = (w)rite, (b)inary
//
//        // Error checking
//        if(lvlFile == NULL) { CGameEngine::fatalError("Failed to open " + data); }
//
//        // file size
//        fseek (lvlFile , 0 , SEEK_END); // set position to end
//        lSize = ftell (lvlFile); // get size in bytes
//        rewind (lvlFile); // set position to beginning
//        buffer.resize(lSize); // adjust buffer to size of file
//
//        // validate data & close file
//        size_t result = fread(&buffer[0], 1, lSize, lvlFile); // read data
//        if(result != lSize) { CGameEngine::fatalError("Read size (" + std::to_string(lSize) + "), expected size (" + std::to_string(result) + ") on file [" + data + "]"); }
//        fclose(lvlFile);
//    }
//    else
//    {
//        lSize = sizeof(data);
//
//         // break up into lines
//        buffer.resize(sizeof(data));
//        buffer = data;
//    }
//
//
//    while(pos < lSize-1)
//    {
//        int newPos = buffer.find('\n', pos); // find index of next "\n" occurrence
//        if( (newPos - pos) > 3) // should never happen
//        {
//            std::string tmp = buffer.substr(pos, newPos-pos); // cut out new string and newline
//            //trim(tmp); // cut off newlines
//            tmp.erase(0, tmp.find_first_not_of(trimThese)); // left trim
//            tmp.erase(tmp.find_last_not_of(trimThese) + 1); // right trim
//            lines.push_back(tmp); // add to vector
//        }
//        pos = newPos+1; // note new position
//    }
//}
