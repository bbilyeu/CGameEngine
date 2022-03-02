#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H
#include <map>
///#include <tuple>
#include "common/types.h"
#include "draw/Texture2D.h"

#define MAX_NUM_SLICES 64

/*
	TextureContainer is a texture and its mipmap levels (Texture2D).
	Texture2D is a 'slice' of TextureContainer, which represents one mipmap level each.
	TextureManager is the texture caching utility and texture array holder.
*/

/// \TODO: Correct logic for ensuring loaded textures are not reloaded
/// \TODO: Add method to raise m_textureArrayLevels as needed, rebinding where necessary. (if possible)

//static bool hasGLExtension(std::string ext);

namespace CGameEngine
{
    class TextureManager
    {
		public:
			// allow access without needing to manually declare/pass-by-ref
			static TextureManager& getInstance() { static TextureManager instance; return instance; }
			bool init();
			void dispose();
			Texture2D* getTexture(std::string filePath);
			Texture2D* newTexture2D(GLsizei mipLevels, GLenum internalFormat, GLsizei width, GLsizei height, std::string name = ""); // create new texture, returning ptr to the blank texture2d object
			Texture2D* loadDDS(std::string filePath); // pulling in data from DDS
			Texture2D* loadPNG(std::string filePath); // pulling in data from PNG
			TextureContainer* newTextureContainer(GLsizei mipLevels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei numSlices);
			void free(Texture2D* tx) { safeDelete(tx); }

		private:
			TextureManager();
			~TextureManager();
			TextureContainer* allocateTextureContainer(GLsizei mipLevels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei numSlices);
			Texture2D* allocateTexture2D(GLsizei mipLevels, GLenum internalFormat, GLsizei width, GLsizei height, std::string = "");

			std::map<std::tuple<GLsizei, GLenum, GLsizei, GLsizei>, std::vector<TextureContainer*>> m_texture2DArrays;
			std::map<std::string, Texture2D*> m_textureMap;
			GLsizei m_maxTextureArrayLevels = 0;
			GLsizei m_textureArrayLevels = 0;
			bool m_init = false;
    };
}
#endif // TEXTUREMANAGER_H
