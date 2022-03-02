#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H
//#include "draw/TextureManager.h"
#include <string>

namespace CGameEngine
{
    /// \TODO: Clean this up. Can everything be condense into IOManager or ResourceManager, fixing TextureCache loading spaghetti
    class ResourceManager
    {
        public:
            //static Texture2D* getTexture(std::string texturePath, bool noRepeat = true);
            //static std::string getPathByTextureID(GLuint texID);

        private:
            //static TextureCache m_textureCache;
    };
}

#endif // RESOURCEMANAGER_H
