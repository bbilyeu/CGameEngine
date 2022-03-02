#ifndef TILESHEET_H_INCLUDED
#define TILESHEET_H_INCLUDED

#include "common/types.h"
#include "draw/Texture2D.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>

/*
    http://rapidjson.org/md_doc_tutorial.html

*/

/// \TODO: Can TileSheets accurately provide the destRect?
/// \TODO: Finish TileSheet loading
/// \TODO: Change SafeVector to SafeUnorderedMap

namespace CGameEngine
{
    struct Tile
    {
        std::string name = "";
        glm::vec4 destRect = glm::vec4(0.0f);
        glm::vec4 uvRect = glm::vec4(0.0f);
        bool rotated = false;
    };

    /// \TODO: Determine if this can be header-only
    class TileSheet
    {
        public:
            TileSheet() {}
            TileSheet(std::string texturePath, std::string jsonPath, std::string label = "NULL");
            ~TileSheet() { m_tiles.clear(); }
            const Texture2D* getTexture() const { return m_texture; }
            TextureAddress getTextureAddress() { return m_texture->getAddress(); }
            const std::string& getFilePath() const { return m_filePath; }
            const std::string& getJSONPath() const { return m_jsonPath; }
            const std::string& getLabel() const { return m_label; }
            std::vector<Tile> getTilesByName(std::string str);
            const std::vector<Tile>& getAllTiles() const { return m_tiles; }

            // these are broken functions that need to be removed from GUI
            const int getIndexByUV(glm::vec4& uv) { return 0; }
            const uint8_t getMaxIndex() const { return 1; }
            const glm::vec4 getUVs(int index) { return glm::vec4(0.0f); } // get the uv coordinates of the sprite we want

        private:
            Texture2D* m_texture; // actual texture "sheet" image
            std::string m_filePath = "";
            std::string m_jsonPath = "";
            std::string m_label = "NULL";
            std::vector<Tile> m_tiles;
    };
}
#endif // TILESHEET_H_INCLUDED
