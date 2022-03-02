#include "draw/Texture2D.h"
#include "srv/Logger.h"
#include "common/util.h"

namespace CGameEngine
{
	/// Texture2D public functions ////////////////////////////////////////////

	Texture2D::~Texture2D() { free(); }
	void Texture2D::commit() { m_container->commit(this); }
	void Texture2D::free() { m_container->free(this); }
	TextureContainer* Texture2D::getContainer() { return m_container; }
	GLsizei Texture2D::getSliceNumber() const { return m_sliceNumber; }

	/**
	 * @brief Construct a new Texture 2D :: Texture 2D object
	 *
	 * @param ta texture container to receive new texture
	 * @param sliceNumber the specific layer to which it will reside
	 * @param name string identifier (not currently used well)
	 */
	Texture2D::Texture2D(TextureContainer* ta, GLsizei sliceNumber, std::string name /*= ""*/)
		: m_container(ta), m_sliceNumber(sliceNumber), m_name(name)
	{
		if(!m_container) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "Texture2D::Texture2D()", "Passed nullptr for container."); }
	}

	/**
	 * @brief trigger setTexSubImage3D against the container to specify the container layer (slice)
	 *
	 * @param mipLevel which mip level is being used
	 * @param xOffset offset from x position on the container/slice
	 * @param yOffset offset from y position on the container/slice
	 * @param width width in pixels
	 * @param height height in pixels
	 * @param internalFormat openGL image format type
	 * @param imageSize size of the image (bytes, post openGL math)
	 * @param data image data in raw form
	 */
	void Texture2D::setTexSubImage2D(GLint mipLevel, GLint xOffset, GLint yOffset, GLsizei width, GLsizei height, GLenum internalFormat, GLsizei imageSize, const GLvoid* data)
	{
		//Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "Texture2D::setTexSubImage2D()", "mipLevel [{}], xOffset [{}], yOffset [{}], sliceNumber [{}], width [{}], height [{}], depth [1], internalFormat [{}], imageSize [{}], data [{}]", mipLevel, xOffset, yOffset, getSliceNumber(), width, height, glFormatToString(internalFormat), imageSize, data);
		m_container->setTexSubImage3D(mipLevel, xOffset, yOffset, getSliceNumber(), width, height, 1, internalFormat, imageSize, data);
	}

	/**
	 * @brief get the address (container and slice numbers) of a texture
	 *
	 * @return TextureAddress container and slice numbers of the texture
	 */
	TextureAddress Texture2D::getAddress() const
	{
//		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "Texture2D::getAddress()", "is container null? [{}]  container mem address? [{}]", (bool)(m_container == nullptr), (void*)m_container);
		TextureAddress addr = { (uint64_t)m_container->getHandle(), m_sliceNumber };
		return addr;
	}

	const int Texture2D::getWidth() const { return (int)(m_container->getWidth()); }
	const int Texture2D::getHeight() const { return (int)(m_container->getHeight()); }

	/// TextureContainer public functions ///////////////////////////////////
    TextureContainer::~TextureContainer()
	{
        glMakeTextureHandleNonResidentARB(m_handle);
        glDeleteTextures(1, &m_textureID);
    }
	GLsizei TextureContainer::hasRoom() const { return m_slots.size() > 0; }
	GLsizei TextureContainer::virtualAllocate() { GLsizei retVal = m_slots.front(); m_slots.pop(); return retVal; }
	void TextureContainer::virtualFree(GLsizei sliceNumber) { m_slots.push(sliceNumber); }
	void TextureContainer::commit(Texture2D* tx) { if(tx->getContainer() == this) { } }
	void TextureContainer::free(Texture2D* tx) { if(tx->getContainer() == this) { } }
	const GLuint64& TextureContainer::getHandle() const { return m_handle; }

	/**
	 * @brief Construct a new Texture Container :: Texture Container object
	 *
	 * @param mipLevels number of mip levels
	 * @param internalFormat openGL image format type
	 * @param width width in pixels
	 * @param height height in pixels
	 * @param numSlices number of layers in the container
	 */
	TextureContainer::TextureContainer(GLsizei mipLevels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei numSlices)
		: m_internalFormat(internalFormat), m_width(width), m_height(height), m_mipLevels(mipLevels), m_slices(numSlices)
	{
		glGenTextures(1, &m_textureID); // get texture ID
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID);
		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "Texture2D::TextureContainer()", "mipLevels [{}], internalFormat [{}], width [{}], height [{}], numSlices [{}]", m_mipLevels, glFormatToString(internalFormat), m_width, m_height, m_slices);

		// ensure we're not over max
		GLsizei maxSlices;
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, maxSlices);
		if(m_slices >= maxSlices)
		{
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, m_slices);
            Logger::getInstance().Log(Logs::WARN, Logs::Drawing, "Texture2D::TextureContainer()", "Attempted to create container of '{}' slices, though the maximum is '{}'. Using maximum instead.", numSlices, maxSlices);
		}

		// this command defines the internal format (e.x. GL_RGBA8, GL_RGBA16, etc)
		glTexStorage3D(GL_TEXTURE_2D_ARRAY, m_mipLevels, m_internalFormat, m_width, m_height, m_slices);
		for (GLsizei i = 0; i < m_slices; ++i) { m_slots.push(i); }

		m_handle = glGetTextureHandleARB(m_textureID); // get GPU-visible address handle
		if(m_handle == 0) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "TextureContainer::TextureContainer()", "Handle is 0, glGetTextureHandleARB failed."); }
		glMakeTextureHandleResidentARB(m_handle); // pass residency to GPU (where textures 'live')
	}

	/**
	 * @brief
	 *
	 * @param mipLevel which mip level is being used
	 * @param xOffset offset from x position on the container/slice
	 * @param yOffset offset from y position on the container/slice
	 * @param sliceNumber which layer this texture belongs
	 * @param width width in pixels
	 * @param height height in pixels
	 * @param depth distance from screen 'front'
	 * @param internalFormat openGL image format type
	 * @param imageSize size of the image (bytes, post openGL math)
	 * @param data image data in raw form
	 * @return true successfully set the texture to the specifed layer
	 * @return false failed to set the texture
	 */
	bool TextureContainer::setTexSubImage3D(GLint mipLevel, GLint xOffset, GLint yOffset, GLint sliceNumber, GLsizei width, GLsizei height, GLsizei depth, GLenum internalFormat, GLsizei imageSize, const GLvoid* data)
	{
		/// \TOOD: Convert to bool, check for failures
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureID);

//        switch (internalFormat)
//        {
//            case GL_RGBA32F:
//            case GL_RGBA16F:
//                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipLevel, xOffset, yOffset, sliceNumber, width, height, depth, internalFormat, GL_FLOAT, data);
//                break;
//            case GL_RGBA32UI:
//            case GL_RGBA16UI:
//            case GL_RGBA8UI:
//                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipLevel, xOffset, yOffset, sliceNumber, width, height, depth, internalFormat, GL_UNSIGNED_INT, data);
//                break;
//            case GL_RGBA32I:
//            case GL_RGBA16I:
//            case GL_RGBA8I:
//                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipLevel, xOffset, yOffset, sliceNumber, width, height, depth, internalFormat, GL_INT, data);
//                break;
//            default:
//                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipLevel, xOffset, yOffset, sliceNumber, width, height, depth, internalFormat, GL_UNSIGNED_BYTE, data);
//                break;
//        }

        // Specifies the symbolic format (GL_RED, GL_RG, GL_RGB, GL_BGR, GL_RGBA, GL_DEPTH_COMPONENT, or GL_STENCIL_INDEX)
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipLevel, xOffset, yOffset, sliceNumber, width, height, depth, internalFormat, GL_UNSIGNED_BYTE, data);

		/// \TODO: Write better method for determining if compressed format or not (potentially use case with joined branches?)
//		if(internalFormat == GL_RGBA || internalFormat == GL_RGB || internalFormat == GL_RGB16F || internalFormat == GL_RGBA16F || internalFormat == GL_RGB32F || internalFormat == GL_RGBA32F)
//		{
//			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipLevel, xOffset, yOffset, sliceNumber, width, height, depth, internalFormat, GL_UNSIGNED_BYTE, data);
//		}
//		else { glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipLevel, xOffset, yOffset, sliceNumber, width, height, depth, internalFormat, imageSize, data); }
//		if(GLenum err = glGetError() != GL_NO_ERROR) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "TextureContainer::setTexSubImage3D()", "glGetError returned [{}] after glCompressedTexSubImage3D", getGLErrorString(err)); return false; }

		// else
		return true;
	}
}
