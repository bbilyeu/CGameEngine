#include "draw/TileSheet.h"
#include "srv/IOManager.h"
//#include "srv/ResourceManager.h"
#include "draw/TextureManager.h"
#include "srv/ConfigReader.h"

namespace CGameEngine
{
    TileSheet::TileSheet(std::string texturePath, std::string jsonPath, std::string label /*= "NULL"*/) : m_filePath(texturePath), m_jsonPath(jsonPath), m_label(label)
    {
        /// \TODO: load texture into memory
        if(!IOManager::doesFileExist(texturePath)) { Logger::getInstance().Log(Logs::FATAL, "TileSheet::TileSheet()", "The texture at path '{}' does not exist!", texturePath); }
        else if(!IOManager::doesFileExist(jsonPath)) { Logger::getInstance().Log(Logs::FATAL, "TileSheet::TileSheet()", "The JSON file at path '{}' does not exist!", jsonPath); }

        // load assets
        m_texture = TextureManager::getInstance().getTexture(texturePath);
		CGameEngine::ConfigReader* config = new CGameEngine::ConfigReader(jsonPath);

		if(config->hasName("frames"))
		{
			std::vector<std::string> tiles = config->getMembers("frames");
			if(tiles.size() == 0) { Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "TileSheet::TileSheet()", "Found zero tiles under the 'frames' object!"); }
			else
			{
                for(unsigned int i = 0; i < tiles.size(); i++)
                {
					std::string tmpPath = "frames/" + tiles[i];
					Tile t;
					t.name = tiles[i];
					if(	config->getValue(std::string(tmpPath + "/destRect"), t.destRect) &&
						config->getValue(std::string(tmpPath + "/uvRect"), t.uvRect) &&
						config->getValue(std::string(tmpPath + "/rotated"), t.rotated))
					{
						m_tiles.push_back(t);
					}
					else
					{
						Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "TileSheet::TileSheet()", "Critical failure loading tiles! name [{}], destRect [{},{},{},{}], uvRect [{},{},{},{}], rotated [{}]", t.name, t.destRect.x, t.destRect.y, t.destRect.z, t.destRect.w, t.uvRect.x, t.uvRect.y, t.uvRect.z, t.uvRect.w, t.rotated);
					}

                }
			}
		}


//        // get json data
//        rapidjson::Document document;
//        document.Parse(buf.c_str());
//        const rapidjson::Value& frames = document["frames"];
//
//        // iterate through each "frame" object
//        for(rapidjson::Value::ConstValueIterator it = frames.Begin(); it = frames.End(); ++it)
//        {
//            Tile t;
//            t.name = (*it)["name"].GetString();
//            t.destRect = glm::vec4((*it)["destRect"][0].GetInt(), (*it)["destRect"][1].GetInt(), (*it)["destRect"][2].GetInt(), (*it)["destRect"][3].GetInt());
//            t.uvRect = glm::vec4((*it)["uvRect"][0].GetDouble(), (*it)["uvRect"][1].GetDouble(), (*it)["uvRect"][2].GetDouble(), (*it)["uvRect"][3].GetDouble());
//            t.rotated = (*it)["rotated"].GetBool();
//            m_tiles.push_back(t);
//        }
    }

    std::vector<Tile> TileSheet::getTilesByName(std::string str)
    {
        std::vector<Tile> retVal;
        for(unsigned int i = 0; i < m_tiles.size(); i++)
        {
            // search for similar names
            if(m_tiles[i].name.find(str) != std::string::npos)
            {
                retVal.push_back(m_tiles[i]);
            }
        }

        return retVal;
    }
}
