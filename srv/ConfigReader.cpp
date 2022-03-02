#include "ConfigReader.h"
#include "sys/stat.h"
#include <stdio.h>
#include "rapidjson/filereadstream.h"
#include "rapidjson/writer.h"

namespace CGameEngine
{
    ConfigReader::ConfigReader(std::string config, bool isFile /*= true*/) : m_path(config)
    {
		if(isFile)
		{
			struct stat s;
			if(stat(m_path.c_str(), &s) == 0) // file exists
			{
				FILE* cnf = fopen(m_path.c_str(), "r");
				if(cnf != NULL)
				{
					char* buffer = new char[m_bufferLength];
					memset(buffer, 0, m_bufferLength);
					rapidjson::FileReadStream fs(cnf, buffer, m_bufferLength);
					m_doc.ParseStream(fs);
					fclose(cnf);
				}
				else
				{
					Logger::getInstance().Log(Logs::FATAL, Logs::IO, "ConfigReader::ConfigReader(std::string)", "File path [{}] exists, but fopen failed!", m_path);
					return; // useless
				}
			}
			else // file does NOT exist
			{
				Logger::getInstance().Log(Logs::CRIT, Logs::IO, "ConfigReader::ConfigReader(std::string)", "File path [{}] does not exist!", m_path);
				return;
			}
        }
        else // raw json data
        {
			m_doc.Parse(config.c_str());
        }
    }

	// for strings by JSONpointers
	bool ConfigReader::getValue(std::string location, std::string& retVal)
	{
		rapidjson::Value* v = getPointer(location);
		if(v && v != nullptr) { retVal = v->GetString(); return true; } else { return false; }
	}

	// for strings by ['category'].member
	bool ConfigReader::getValue(std::string category, std::string member, std::string& retVal)
	{
		rapidjson::Value* v = (hasName(category, member)) ? &m_doc[category.c_str()][member.c_str()] : nullptr;
		if(v && v != nullptr) { retVal = v->GetString(); return true; } else { return false; }
	}

	// for vec2 by JSONpointers
	bool ConfigReader::getValue(std::string location, glm::vec2& retVal)
	{
		rapidjson::Value* v = getPointer(location);
		if(v && v != nullptr && v->IsArray() && v->Size() == 2)
		{
			int counter = 0;
			for(auto& f : v->GetArray()) { retVal[counter] = f.GetFloat(); counter++; }
			return true; // success
		}
		// else
		return false;
	}

	// for vec2 by ['category'].member
	bool ConfigReader::getValue(std::string category, std::string member, glm::vec2& retVal)
	{
		rapidjson::Value* v = (hasName(category, member)) ? &m_doc[category.c_str()][member.c_str()] : nullptr;
		if(v && v != nullptr && v->IsArray() && v->Size() == 2)
		{
			int counter = 0;
			for(auto& f : v->GetArray()) { retVal[counter] = f.GetFloat(); counter++; }
			return true; // success
		}
		// else
		return false;
	}

	// for vec4 by JSONpointers
	bool ConfigReader::getValue(std::string location, glm::vec4& retVal)
	{
		rapidjson::Value* v = getPointer(location);
		if(v && v != nullptr && v->IsArray() && v->Size() == 4)
		{
			int counter = 0;
			for(auto& f : v->GetArray()) { retVal[counter] = f.GetFloat(); counter++; }
			return true; // success
		}
		// else
		return false;
	}

	// for vec4 by ['category'].member
	bool ConfigReader::getValue(std::string category, std::string member, glm::vec4& retVal)
	{
		rapidjson::Value* v = (hasName(category, member)) ? &m_doc[category.c_str()][member.c_str()] : nullptr;
		if(v && v != nullptr && v->IsArray() && v->Size() == 4)
		{
			int counter = 0;
			for(auto& f : v->GetArray()) { retVal[counter] = f.GetFloat(); counter++; }
			return true; // success
		}
		// else
		return false;
	}

	// for string array types by JSONpointers
	std::vector<std::string> ConfigReader::getArrayValue(std::string location)
	{
		std::vector<std::string> retVal;
		rapidjson::Value* v = getPointer(location);
		if(v && v != nullptr) { for(auto& i : v->GetArray()) { retVal.push_back(i.GetString()); } }
		return retVal;
	}

	// for string array types by ['category'].member
	std::vector<std::string> ConfigReader::getArrayValue(std::string category, std::string member)
	{
		std::vector<std::string> retVal;
		rapidjson::Value* v = (hasName(category, member)) ? &m_doc[category.c_str()][member.c_str()] : nullptr;
		if(v && v != nullptr) { for(auto& i : v->GetArray()) { retVal.push_back(i.GetString()); } }
		return retVal;
	}

	// for getting vector of vec2s (e.g. polygon shapes)
	std::vector<glm::vec2> ConfigReader::getVec2Array(std::string location)
	{
		/*std::vector<std::vector<glm::vec2>> retVal = std::vector<std::vector<glm::vec2>>();
		rapidjson::Value* v = getPointer(location);
		if(v && v != nullptr)
		{
			retVal.resize(v->MemberCount());
			int c1 = 0, c2 = 0, c3 = 0; // counters
			for(auto& i : v->GetObject()) // from m_doc["level_data"]["terrain"]["a"]
			{
				// from m_doc["level_data"]["terrain"]["a"]["1"]
				const rapidjson::Value &dataVector = i.value;
				if(dataVector.IsArray()) // confirm vector to avoid hard to discern crashes
				{
					// resize current 'row'
					retVal[c1].resize(dataVector.Size());
					for(int j = 0; j < dataVector.Size(); j++)
					{
						glm::vec2 tmpVec = glm::vec2(0.0f); // junk variable
						for(auto& a : dataVector[j].GetArray()) { tmpVec[c3] = a.GetFloat(); c3++; } // populate x/y
						retVal[c1][c2] = tmpVec; // add to vector

						// reset counters
						c2++;
						c3 = 0;
					}

					// reset counters
					c1++;
					c2 = 0;
				}
			}
		}
		return retVal;*/

		std::vector<glm::vec2> retVal = std::vector<glm::vec2>();
		rapidjson::Value* v = getPointer(location);
		if(v && v != nullptr && v->IsArray())
		{
			int arrSize = v->Size();
			for(auto& val : v->GetArray())
			{
				int c = 0; // counter
				glm::vec2 tmpVec = glm::vec2(0.0f); // junk variable
				for(auto& a : val.GetArray()) { tmpVec[c] = a.GetFloat(); c++; } // populate x/y
				retVal.push_back(tmpVec); // add to vector
			}
		}

		return retVal;
	}

	std::vector<std::string> ConfigReader::getMembers(std::string location /*= ""*/)
    {
		std::vector<std::string> symbols;
		if(location == "") { for (auto& m : m_doc.GetObject()) { symbols.push_back(m.name.GetString()); } } // top level, no category
		else if(location.substr(0,1) == "/")  // JSON Pointers : /frames/menu/position
		{
			rapidjson::Value* v = (hasName(location)) ? rapidjson::Pointer(location.c_str()).Get(m_doc) : nullptr;
			if(v && v != nullptr) { for (auto& m : v->GetObject()) { symbols.push_back(m.name.GetString()); } }
			else { throwError(Logs::WARN, "ConfigReader::getMembers()", fmt::format("Location '{}' does not exist or 'v' returned a nullptr.", location)); }
		}
		else // m_doc["somestring"]
		{
			if(m_doc.HasMember(location.c_str())) { for (auto& m : m_doc.GetObject()) {symbols.push_back(m.name.GetString()); } }
			else { throwError(Logs::WARN, "ConfigReader::getMembers()", fmt::format("'{}' does not exist as a member.", location)); }
		}
		return symbols;
    }

	bool ConfigReader::isArray(std::string location)
	{
		return (rapidjson::Pointer(location.c_str()).IsValid() && rapidjson::Pointer(location.c_str()).Get(m_doc)->IsArray());
	}

	bool ConfigReader::isArray(std::string category, std::string member)
	{
		return (m_doc[category.c_str()][member.c_str()].IsArray());
	}

	bool ConfigReader::hasName(std::string location)
	{
		return rapidjson::Pointer(location.c_str()).IsValid();
	}

    bool ConfigReader::hasName(std::string category, std::string member)
    {
		return ( (category == "" && m_doc.HasMember(member.c_str())) || ( m_doc.HasMember(category.c_str()) && m_doc[category.c_str()].HasMember(member.c_str())) );
    }

	/// ConfigReader private functions ////////////////////////////////////////////

	void ConfigReader::throwError(uint16_t lvl, std::string func, std::string errMsg)
	{
		if(m_noFail) { lvl = Logs::FATAL; }
		Logger::getInstance().Log(lvl, Logs::IO, func, errMsg.c_str());
	}

	rapidjson::Value* ConfigReader::getRJValue(std::string category, std::string member)
	{
        if(hasName(category, member)) { return &m_doc[category.c_str()][member.c_str()]; }

        // else
		throwError(Logs::WARN, "ConfigReader::getRJValue()", fmt::format("Failed to find member '{}' in category '{}'.", member, category));
		return nullptr;
	}

	// included catch to allow "somename" to function as "/somename"
	rapidjson::Value* ConfigReader::getPointer(std::string location)
	{
		rapidjson::Value* retVal = (hasName(location)) ? rapidjson::Pointer(location.c_str()).Get(m_doc) : nullptr;
		if(retVal == nullptr) { location = "/" + location; retVal = (hasName(location)) ? rapidjson::Pointer(location.c_str()).Get(m_doc) : nullptr; }
		return retVal;
	}
}
