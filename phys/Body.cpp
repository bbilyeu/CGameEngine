#include "Body.h"

namespace CGameEngine
{
    Body::~Body()
    {
        if(m_body)
        {
            //if(m_fixture) { m_body->DestroyFixture(m_fixture); }
            m_body->GetWorld()->DestroyBody(m_body);
        }
        m_fixture = nullptr;
        m_body = nullptr;
    }

    bool Body::hasContact(int sensor)
    {
        if(m_sensors.empty()) { return false; }

        // check against sensor for -1 (f) or 1 (t)
        auto itr = m_sensors.find(sensor);
        if(itr != m_sensors.end())
        {
            return ( (*static_cast<char*>(itr->second->GetUserData())) == 't' ? true : false );
        }
        else { return false; }
    }

    bool Body::hasSideContact(bool left /*= true*/)
    {
        if(m_sensors.empty()) { return false; }

        // check against sensor for -1 (f) or 1 (t)
        if(left)
        {
            // check left center
            auto itr = m_sensors.find(Sensors::Left);
            if(itr != m_sensors.end() && (*static_cast<char*>(itr->second->GetUserData())) == 't') // left center has contact
            {
                itr = m_sensors.find(Sensors::TopLeft);
                if(itr != m_sensors.end() && (*static_cast<char*>(itr->second->GetUserData())) == 't') { return true; }

                itr = m_sensors.find(Sensors::BottomLeft);
                if(itr != m_sensors.end() && (*static_cast<char*>(itr->second->GetUserData())) == 't') { return true; }
            }
        }
        else // right
        {
            auto itr = m_sensors.find(Sensors::Right);
            if(itr != m_sensors.end() && (*static_cast<char*>(itr->second->GetUserData())) == 't') // right center has contact
            {
                itr = m_sensors.find(Sensors::TopRight);
                if(itr != m_sensors.end() && (*static_cast<char*>(itr->second->GetUserData())) == 't') { return true; }

                itr = m_sensors.find(Sensors::BottomRight);
                if(itr != m_sensors.end() && (*static_cast<char*>(itr->second->GetUserData())) == 't') { return true; }
            }
        }

        // else
        return false;
    }

    // bitwise combine the return value
    const int Body::getContacts() const
    {
        if(m_sensors.empty()) { return 0; }

        int retVal = 0;
        for(auto itr = m_sensors.begin(); itr != m_sensors.end(); ++itr)
        {
            if((*static_cast<char*>(itr->second->GetUserData())) == 't') { retVal |= itr->first; }
        }

        return retVal;
    }

	const glm::vec2 Body::getPosition() const
    {
        b2Vec2 pos = m_body->GetPosition();
        glm::vec2 vPos = glm::vec2(pos.x - (m_dimensions.x / 2.0f), pos.y - (m_dimensions.y / 2.0f));
        return vPos;
    }

    const glm::vec4 Body::getDestRect() const
    {
		glm::vec4 dr;
		b2Vec2 pos = m_body->GetPosition();
		dr.x = static_cast<float>(pos.x - m_dimensions.x / 2.0f);
		dr.y = static_cast<float>(pos.y - m_dimensions.y / 2.0f);
		dr.z = m_dimensions.x;
		dr.w = m_dimensions.y;
		return dr;
    }
}
