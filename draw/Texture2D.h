#ifndef GLTEXTURE_H_INCLUDED
#define GLTEXTURE_H_INCLUDED
#include "common/types.h"
#include <tuple>
#include <queue>

/*
	TextureContainer is a texture and its mipmap levels (Texture2D).
	Texture2D is a 'slice' of TextureContainer, which represents one mipmap level each.
	TextureManager is the texture caching utility and texture array holder.

	Ref:
        target and format breakdown : https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTextureView.xhtml
            Also : https://www.khronos.org/opengl/wiki/Image_Format
*/

/// \TODO: How to free/commit things to texture array?

namespace CGameEngine
{
    static std::string glFormatToString(GLenum format)
    {
        std::string retVal = "NULL";

        switch (format)
        {
            case GL_RGBA: retVal = "GL_RGBA"; break;
            case GL_RGBA8: retVal = "GL_RGBA8"; break;
            case GL_RGBA8I: retVal = "GL_RGBA8I"; break;
            case GL_RGBA8UI: retVal = "GL_RGBA8UI"; break;
            case GL_RGBA16: retVal = "GL_RGBA16"; break;
            case GL_RGBA16F: retVal = "GL_RGBA16F"; break;
            case GL_RGBA16I: retVal = "GL_RGBA16I"; break;
            case GL_RGBA16UI: retVal = "GL_RGBA16UI"; break;
            case GL_RGBA32F: retVal = "GL_RGBA32F"; break;
            case GL_RGBA32I: retVal = "GL_RGBA32I"; break;
            case GL_RGBA32UI: retVal = "GL_RGBA32UI"; break;
            default: retVal = "UNKNOWN"; break;
        }
        return retVal;
    }

	class Texture2D;
	class TextureContainer;

	// a mipmapped iteration of a texture
    class Texture2D
    {
		public:
			Texture2D(TextureContainer* tc, GLsizei sliceNumber, std::string name = "");
            ~Texture2D();
            void commit();
            void free();
            void setTexSubImage2D(GLint mipLevel, GLint xOffset, GLint yOffset, GLsizei width, GLsizei height, GLenum internalFormat, GLsizei imageSize, const GLvoid* data);
			TextureContainer* getContainer();
			GLsizei getSliceNumber() const;
			TextureAddress getAddress() const;
			const int getWidth() const;
			const int getHeight() const;
			const size_t& getSignature() const { return m_imageSignature; }
			inline bool operator==(const Texture2D* other) { return (this->m_container != nullptr && this->getAddress() == other->getAddress()); }
			inline bool operator!=(const Texture2D* other) { return (this->m_container != nullptr && this->getAddress() != other->getAddress()); }

		private:
			TextureContainer* m_container = nullptr;
			GLsizei m_sliceNumber = 0; // array index
			std::string m_name; // or filePath
			size_t m_imageSignature = 0;
    };

	// contains a texture and its mipmap levels
	class TextureContainer
    {
		public:
			TextureContainer(GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei numSlices);
			~TextureContainer();
			GLsizei hasRoom() const;
			GLsizei virtualAllocate();
			void virtualFree(GLsizei sliceNumber);
			void commit(Texture2D* tx);
            void free(Texture2D* tx);
            bool setTexSubImage3D(GLint mipLevel, GLint xOffset, GLint yOffset, GLint sliceNumber, GLsizei width, GLsizei height,
							GLsizei depth, GLenum internalFormat, GLsizei imageSize, const GLvoid* data);
			const GLuint64& getHandle() const;
			const GLsizei& getWidth() const { return m_width; }
			const GLsizei& getHeight() const { return m_height; }
			const GLenum& getFormat() const { return m_internalFormat; }

		private:
			GLuint64 m_handle = 0;
			GLuint m_textureID = 0;
			std::queue<GLsizei> m_slots;
			const GLsizei m_width;
			const GLsizei m_height;
			const GLsizei m_mipLevels;
			const GLsizei m_slices;
			const GLenum m_internalFormat;
			GLsizei m_tileSizeX = 0;
			GLsizei m_tileSizeY = 0;

			void changeCommit(GLsizei sliceNumber, GLboolean commit);
    };
}

#endif
