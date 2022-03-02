#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "vector"
#include "common/util.h"
#include "glm/glm.hpp"
#include "srv/Logger.h"

/*
    Ref:
        https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
        http://rapidjson.org/index.html
        http://www.cplusplus.com/doc/tutorial/files/
        https://eli.thegreenplace.net/2014/sfinae-and-enable_if/	-- enable_if
        https://stackoverflow.com/questions/37443829/processing-arrays-of-arrays-of-integer-with-rapidjson (array of arrays)
*/

namespace CGameEngine
{
    class ConfigReader
    {
        public:
            ConfigReader(std::string config, bool isFile = true);
            virtual ~ConfigReader() {}

			// for integral types by JSONpointers
			template <typename T>
			bool getValue(std::string location, T& retVal)
            {
				rapidjson::Value* v = getPointer(location);
				return getIntegralValue(v, retVal);
            }

            // for integral types by ['category'].member
			template <typename T>
            bool getValue(std::string category, std::string member, T& retVal)
            {
				rapidjson::Value* v = (hasName(category, member)) ? &m_doc[category.c_str()][member.c_str()] : nullptr;
				return getIntegralValue(v, retVal);
            }

			// for integral array types by JSONpointers
			template <typename T>
            std::vector<T> getArrayValue(std::string location)
            {
				rapidjson::Value* v = getPointer(location);
				std::vector<T> retVal;
				getArrayValue(v, retVal);
				return retVal;
            }

            // for integral array types by ['category'].member
			template <typename T>
            std::vector<T> getArrayValue(std::string category, std::string member)
            {
				rapidjson::Value* v = (hasName(category, member)) ? &m_doc[category.c_str()][member.c_str()] : nullptr;
				std::vector<T> retVal;
				getArrayValue(v, retVal);
				return retVal;
            }

			// for strings by JSONpointers
			bool getValue(std::string location, std::string& retVal);

            // for strings by ['category'].member
            bool getValue(std::string category, std::string member, std::string& retVal);

            // for vec2 by JSONpointers
            bool getValue(std::string location, glm::vec2& retVal);

            // for vec2 by ['category'].member
            bool getValue(std::string category, std::string member, glm::vec2& retVal);

            // for vec4 by JSONpointers
            bool getValue(std::string location, glm::vec4& retVal);

            // for vec4 by ['category'].member
            bool getValue(std::string category, std::string member, glm::vec4& retVal);

            // for string array types by JSONpointers
            std::vector<std::string> getArrayValue(std::string location);

            // for string array types by ['category'].member
            std::vector<std::string> getArrayValue(std::string category, std::string member);

            // for getting vector of vec2s (e.g. polygon shapes)
            std::vector<glm::vec2> getVec2Array(std::string location);

            // return vector of members (as strings)
            std::vector<std::string> getMembers(std::string category = "");

            bool isArray(std::string location);

            bool isArray(std::string category, std::string member);


            std::string& getConfigPath() { return m_path; }
            rapidjson::Document* getDocument() { return &m_doc; }
            bool hasName(std::string location);
            bool hasName(std::string category, std::string member);
            bool isValid() { return (!m_doc.HasParseError()); }
            void setNoFail() { m_noFail = true; }


        private:
			bool m_noFail = false; // setting to true throws a fatal error instead of warning on failures
            rapidjson::Document m_doc;
            std::string m_path = "-1";
            const uint16_t m_bufferLength = 8192;
            void throwError(uint16_t lvl, std::string func, std::string errMsg);
            rapidjson::Value* getRJValue(std::string category, std::string member);
            rapidjson::Value* getPointer(std::string location);

			// for integral types with determination
            template <typename T>
			bool getIntegralValue(rapidjson::Value* v, T& retVal)
            {
				if(v && v != nullptr)
				{
					std::string func = "ConfigReader::getIntegralValue(rapidjson::Value*, T&)";
					std::string rType = type_name<decltype(retVal)>();
					if(rType == "bool") { retVal = v->GetBool(); }
					else if(rType == "int" || rType == "int16_t") { retVal = v->GetInt(); }
					else if(rType == "int64") { retVal = v->GetInt64(); }
					else if(rType == "uint32_t" || rType == "uint16_t") { retVal = v->GetUint(); }
					else if(rType == "uint64_t") { retVal = v->GetUint64(); }
					else if(rType == "float") { retVal = v->GetFloat(); }
					else if(rType == "double") { retVal = v->GetDouble(); }
					else
					{
						// unknown type
						throwError(Logs::WARN, func, fmt::format("Unknown type ({}) passed.", rType));
						return false;
					}

					// if we made it this far, we succeeded
					return true;
				}
				// else, we failed
				return false;
            }

            template <typename T>
            void getArrayValue(rapidjson::Value* v, std::vector<T>& retVal)
            {
				if(v && v != nullptr && v->IsArray())
				{
					std::string func = "ConfigReader::getArrayValue(rapidjson::Value*, std::vector<T>&)";
					std::string rType = type_name<decltype(retVal)>();
					if(rType.find("float") != std::string::npos) { for(auto& i : v->GetArray()) { retVal.push_back(i.GetFloat()); } }
					else if(rType.find("int") != std::string::npos)	{ for(auto& i : v->GetArray()) { retVal.push_back(i.GetInt()); } }
					else
					{
						// unknown type
						throwError(Logs::WARN, func, fmt::format("Unknown type ({}) passed.", rType));
						return;
					}
				}
			}

	};
}

#endif // CONFIGREADER_H
