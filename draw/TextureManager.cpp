#include "draw/TextureManager.h"
#include "draw/picoPNG.h"
#include "draw/DDSHelper.h"
#include "srv/IOManager.h"
#include "common/util.h"


/**
 * @brief TextureManager is a Meyer's singleton class that is a texture caching utility and texture array holder.
 *
 * TextureContainer is a texture and its mipmap levels (Texture2D).
 * Texture2D is a 'slice' of TextureContainer, which represents one mipmap level each.
 */

/**
 * @brief checks for passed extensions (e.g. ARB_sparse_texture)
 *
 * source: https://www.opengl.org/archives/resources/features/OGLextensions/
 *
 * @param ext passed openGL extension to check compatibility (e.g. ARB_sparse_texture)
 * @return true the extension exists and is usable
 * @return false the extension doesn't exist or does exist but isn't usable
 */
static bool hasGLExtension(std::string ext)
{
	const GLubyte *extensions = NULL;
	const GLubyte *start;
	GLubyte *where, *terminator;

	/* Extension names should not have spaces. */
	where = (GLubyte *) strchr(ext.c_str(), ' ');
	if(where || ext.size() == 0) { return false; }

    // pre OpenGL 4.0
	//extensions = glGetString(GL_EXTENSIONS);

	/*
		In some cases, like core context, the extensions will be null due
		to a depreciated function. To check, we must use the method below.
	*/
	if (extensions == NULL) {
		GLint n;
		glGetIntegerv(GL_NUM_EXTENSIONS, &n);
		if (n <= 0) {
			Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "hasGLExtension()", "Can not check system extensions");
			return false;
		}
        const char** extensions = (const char**)malloc(n * sizeof(char*));
        GLint i;
        for (i = 0; i < n; i++)
        {
            extensions[i] = (char*)glGetStringi(GL_EXTENSIONS, i);
        }

		char** start = (char**) extensions;
		char *where, *terminator;
		for(;;) {
			where = (char *) strstr(*start, ext.c_str());
			if (!where)
				break;
			terminator = where + strlen(ext.c_str());
			if (where == *start || *(where - 1) == ' ')
			if (*terminator == ' ' || *terminator == '\0')
			{
				Logger::getInstance().Log(Logs::WARN, Logs::Drawing, "hasGLExtension()", "[{}] not found/supported.", ext);
				return false;
			}
			*start = terminator;
		}
		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "hasGLExtension()", "[{}] found/supported.", ext);
		return true;
	}

	/* It takes a bit of care to be fool-proof about parsing the
	OpenGL extensions string. Don't be fooled by sub-strings,
	etc. */
	start = extensions;
	for(;;)
	{
		where = (GLubyte *) strstr((const char *) start, ext.c_str());
		if (!where)
		break;
		terminator = where + strlen(ext.c_str());
		if (where == start || *(where - 1) == ' ')
		if (*terminator == ' ' || *terminator == '\0')
		{
			Logger::getInstance().Log(Logs::WARN, Logs::Drawing, "hasGLExtension()", "[{}] not found/supported.", ext);
			return false;
		}
		start = terminator;
	}

	Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "hasGLExtension()", "[{}] found/supported.", ext);
	return true;
}


namespace CGameEngine
{
	/**
	 * @brief init the TextureManager and check for required extensions
	 *
	 * @return true successfully created the texture manager
	 * @return false failed to create the texture manager, likely fatal
	 */
	bool TextureManager::init()
	{
		// quick exit
		if(m_init) { return true; }

		if(!hasGLExtension("ARB_bindless_texture")) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "TextureManager::init()",	"Exiting as ARB_bindless_texture are not supported!"); }
		if(!hasGLExtension("ARB_multi_draw_indirect")) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "TextureManager::init()", "MultiDrawIndirect not found! ABORTING UNTIL LATER STAGE OF DEVELOPMENT!"); }

		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &m_maxTextureArrayLevels);
		m_textureArrayLevels = (m_maxTextureArrayLevels > MAX_NUM_SLICES) ? MAX_NUM_SLICES : m_maxTextureArrayLevels; // prevent memory maxing
		if(m_maxTextureArrayLevels > 0)
        {
            Logger::getInstance().Log(Logs::INFO, Logs::Drawing, "TextureManager::init()", "Using texture array levels '{}' of max texture array levels '{}'", m_textureArrayLevels, m_maxTextureArrayLevels);
        }
        else
        {
            Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "TextureManager::init()", "Max texture array levels ({}) is 0!", m_maxTextureArrayLevels);
        }

		m_init = true;
		return true;
	}

	/**
	 * @brief public deconstructor-like function
	 */
	void TextureManager::dispose()
	{
		// iterate through the map
		for(auto it = m_texture2DArrays.begin(); it != m_texture2DArrays.end(); ++it)
		{
			// iterate through the vector
			for(auto tupIt = it->second.begin(); tupIt < it->second.end(); ++tupIt)
			{
				// trash the data, set nullptr
				safeDelete(*tupIt);
			}
		}
		m_texture2DArrays.clear();
		m_init = false;
	}

	/**
	 * @brief get texture2D by checking existence, creating if needed
	 *
	 * @param filePath texture's filepath
	 * @return Texture2D* pointer to the texture2D
	 */
	Texture2D* TextureManager::getTexture(std::string filePath)
	{
		/// \TODO: Create logic to deliver existing textures
		Texture2D* retVal = nullptr;

		// check existing textures first
		auto it = m_textureMap.find(filePath);
		if(it != m_textureMap.end() && it->second != nullptr) { retVal = it->second; }

		// otherwise load normally
		else if(doesContain(filePath, "png")) { retVal = loadPNG(filePath); }
		else if(doesContain(filePath, "dds")) { retVal = loadDDS(filePath); }
		else { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "TextureManager::getTexture()", "Could not determine filetype of '{}'!", filePath); }
		return retVal;
	}

	/**
	 * @brief create new texture, returning ptr to the blank texture2d object
	 *
	 * @param mipLevels number of mipmap levels
	 * @param internalFormat openGL internal format type
	 * @param width width in pixels
	 * @param height height in pixels
	 * @param name string for texture name (often filepath)
	 * @return Texture2D* texture2D object being created
	 */
	Texture2D* TextureManager::newTexture2D(GLsizei mipLevels, GLenum internalFormat, GLsizei width, GLsizei height, std::string name /*= ""*/)
	{
		Logger::getInstance().Log(Logs::VERBOSE, Logs::IO, "TextureManager::newTexture2D()", "mipLevels [{}], internalFormat [{}], width [{}], height [{}]", mipLevels, glFormatToString(internalFormat), width, height);
		if(m_init)
		{
			Texture2D* retVal = allocateTexture2D(mipLevels, internalFormat, width, height, name);
			retVal->commit();
			return retVal;
		}
		else { return nullptr; }
	}

	/**
	 * @brief pulling in data from DDS
	 *
	 * @param filePath path to texture in filesystem
	 * @return Texture2D* created texture object
	 */
	Texture2D* TextureManager::loadDDS(std::string filePath)
	{
		Texture2D* retVal = nullptr;
		TextureDetails td;
		size_t offset = 0;

		if(IOManager::doesFileExist(filePath))
		{
			// read in from file
			if(!readDDSFromFile(filePath, &td)) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "TextureManager::loadDDS()", "Failed to process readDDSFromFile() against '{}'", filePath); return nullptr; }

			// create new texture object
			Logger::getInstance().Log(Logs::VERBOSE, Logs::IO, "TextureManager::loadDDS()", "mipLevels '{}', format '{}', width '{}', height '{}'", td.mipmapLevels, glFormatToString(td.glFormat), td.dWidth, td.dHeight);
			retVal = newTexture2D(td.mipmapLevels, td.glFormat, td.dWidth, td.dHeight, filePath);
			if(!retVal)
			{
				Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "TextureManager::loadDDS()", "newTexture2D() returned nullptr. Init status [{}]", m_init);
				safeDelete(retVal);
				return nullptr;
			}

			// creating mip levels
			for(int m = 0; m < td.mipmapLevels; ++m)
			{
				retVal->setTexSubImage2D(m, 0, 0, td.MipMapWidth(m), td.MipMapHeight(m), td.glFormat, td.mipmapSizes[m], (char*)td.imageData + offset);
				offset += td.mipmapSizes[m];
			}

			// store in texture map
			m_textureMap.insert(std::make_pair(filePath, retVal));
		}
		else { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "TextureManager::loadDDS()", "File [{}] not found!", filePath); }

		return retVal;
	}

	/**
	 * @brief pulling in data from PNG
	 *
	 * @param filePath path to texture in filesystem
	 * @return Texture2D* created texture object
	 */
	Texture2D* TextureManager::loadPNG(std::string filePath)
	{
		Texture2D* retVal = nullptr;
		std::vector<unsigned char> in; // input data
		std::vector<unsigned char> out; // output from decodePNG
		unsigned long width, height, mipLevels = 0;

		if(IOManager::doesFileExist(filePath))
		{
			if(!IOManager::readFileToBuffer(filePath, in)) { Logger::getInstance().Log(Logs::FATAL, Logs::IO, "TextureManager::loadPNG()", "Failed to load PNG file at path '{}' to buffer!", filePath); }
			if(decodePNG(out, width, height, &(in[0]), in.size()) > 0) { Logger::getInstance().Log(Logs::FATAL, Logs::IO, "ImageLoader::loadPNG()", "DecodePNG failed on {}.", filePath); }

			/// \TODO: Implement mipmapping levels
			/*// divide down the width and height to find the maximum number of mipmap levels
			//	(this will be the dimension with the LOWEST number)
			unsigned long xLevels = 0, yLevels = 0, currValue = width;
			while(currValue >= 16) { currValue /= 2; xLevels++; }
			currValue = height;
			while(currValue >= 16) { currValue /= 2; yLevels++; }
			mipLevels = (xLevels > yLevels) ? yLevels : xLevels;*/
			if(mipLevels == 0) { mipLevels = 1; } // prevent small textures from failing the test above

			//Logger::getInstance().Log(Logs::VERBOSE, Logs::IO, "TextureManager::loadPNG()", "mipLevels '{}', GL_RGBA16, width '{}', height '{}'", mipLevels, width, height);
			// this may call glTexStorage3D, which requires a sized internal format (GL_RGBA16, GL_RGBA32F, etc)
			retVal = newTexture2D((GLsizei)mipLevels, GL_RGBA16, (GLsizei)width, (GLsizei)height, filePath);
			if(retVal)
			{
				for(int m = 0; m < (int)mipLevels; ++m)
				{
					/// \TODO: is '(GLsizei)sizeof(GLsizei)' acceptable for imageSize?
					// this will call glTexSubImage3D which requires the symbolic format (GL_RED, GL_RG, GL_RGB, GL_BGR, GL_RGBA, GL_DEPTH_COMPONENT, or GL_STENCIL_INDEX)
					//retVal->setTexSubImage2D(m, 0, 0, (GLsizei)width, (GLsizei)height, GL_RGBA, (GLsizei)sizeof(GLSizei), &(out[0]));
					retVal->setTexSubImage2D(m, 0, 0, (GLsizei)width, (GLsizei)height, GL_RGBA, (GLsizei)out.size(), &(out[0]));
				}

				// store in texture map
				m_textureMap.insert(std::make_pair(filePath, retVal));
			}
			else { Logger::getInstance().Log(Logs::FATAL, Logs::IO, "TextureManager::loadPNG()", "Failed newTexture2D call!"); }
		}
		else { Logger::getInstance().Log(Logs::FATAL, Logs::IO, "TextureManager::loadPNG()", "File '{}' does not exist!", filePath); }

		return retVal;
	}

	/**
	 * @brief wrapper to create a texture container based on openGL format
	 *
	 * @param mipLevels number of mipmap levels
	 * @param internalFormat openGL internal texture format
	 * @param width width in pixels
	 * @param height height in pixels
	 * @param numSlices number of slices (a.k.a levels or array elements)
	 * @return TextureContainer* texture container created by allocateTextureContainer
	 */
	TextureContainer* TextureManager::newTextureContainer(GLsizei mipLevels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei numSlices)
	{
		//Logger::getInstance().Log(Logs::VERBOSE, Logs::IO, "TextureManager::newTextureContainer()", "mipLevels [{}], internalFormat [{}], width [{}], height [{}], numSlices [{}]", mipLevels, internalFormat, width, height, numSlices);
		if(m_init)
		{
			TextureContainer* retVal = allocateTextureContainer(mipLevels, internalFormat, width, height, numSlices);
			return retVal;
		}
		else { return nullptr; }
	}

	/// TextureManager Private Functions //////////////////////////////////////

	TextureManager::TextureManager()
	{
		if(!init()) { Logger::getInstance().Log(Logs::FATAL, Logs::IO, "TextureManager::TextureManager", "Failed during init call within default constructor!"); }
	}

	TextureManager::~TextureManager()
	{
		dispose();
	}

	/**
	 * @brief find or create a texture container (called from newTextureContainer)
	 *
	 * @param mipLevels number of mipmap levels
	 * @param internalFormat openGL internal texture format
	 * @param width width in pixels
	 * @param height height in pixels
	 * @param numSlices number of slices (a.k.a levels or array elements)
	 * @return TextureContainer* created texture container
	 */
	TextureContainer* TextureManager::allocateTextureContainer(GLsizei mipLevels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei numSlices)
	{
		TextureContainer* txContainer = nullptr;

		// create tuple as 'key'
		auto textureType = std::make_tuple(mipLevels, internalFormat, width, height);
		//Logger::getInstance().Log(Logs::VERBOSE, Logs::Drawing, "TextureManager::allocateTextureContainer()", "<tuple> mipLevels [{}], internalFormat [{}], width [{}], height [{}], numSlices [{}]", mipLevels, internalFormat, width, height, numSlices);

		// look for an existing array
		auto arrayIt = m_texture2DArrays.find(textureType);
		if (arrayIt == m_texture2DArrays.end())
		{
			m_texture2DArrays[textureType] = std::vector<TextureContainer*>();
			arrayIt = m_texture2DArrays.find(textureType);
			if(arrayIt == m_texture2DArrays.end()) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "TextureManager::allocateTextureContainer()", "Failed to create a 2D Texture array." ); }
		}

		// dig through containers within current array to see if one has room
		for (auto it = arrayIt->second.begin(); it != arrayIt->second.end(); ++it) { if ((*it)->hasRoom()) { txContainer = (*it); break; } }

		// it does not, create a new container for it
		if (txContainer == nullptr)
		{
			if(numSlices > m_maxTextureArrayLevels)
			{
				Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "TextureManager::allocateTextureContainer()", "Attempt to create too large of a container occurred, reducing to maximum allowed value. Requested [{}], Maximum [{}]", numSlices, m_maxTextureArrayLevels);
				numSlices = m_maxTextureArrayLevels;
			}
			txContainer = new TextureContainer(mipLevels, internalFormat, width, height, numSlices);
			arrayIt->second.push_back(txContainer);
		}

		if(!txContainer) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "TextureManager::allocateTextureContainer()", "Failed to find available TextureContainer or create one!"); }

		return txContainer;
	}

	/**
	 * @brief allocating space for a 2D texture to correct container based on allocateTextureContainer
	 *
	 * @param mipLevels number of mip map levels
	 * @param internalFormat openGL internal texture format
	 * @param width width in pixels
	 * @param height height in pixels
	 * @param name internal identifier (usually filepath)
	 * @return Texture2D* created texture 2D object
	 */
	Texture2D* TextureManager::allocateTexture2D(GLsizei mipLevels, GLenum internalFormat, GLsizei width, GLsizei height, std::string name /*= ""*/)
	{
		TextureContainer* txContainer = nullptr;
		txContainer = allocateTextureContainer(mipLevels, internalFormat, width, height, m_textureArrayLevels);

		// failure if nullptr
		if(!txContainer) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "TextureManager::allocateTexture2D()", "Did not receive TextureContainer pointer from allocateTextureContainer!"); }

		// success
		Texture2D* retVal = new Texture2D(txContainer, txContainer->virtualAllocate(), name);
		return retVal;
	}
}
