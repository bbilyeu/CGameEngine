#ifndef SENSORBODY_H_INCLUDED
#define SENSORBODY_H_INCLUDED

#include "phys/Body.h"

namespace CGameEngine
{
    class SensorBody : public Body
    {
        public:
            SensorBody(b2World* world, CGameEngine::Body* hostBody, const float& radius);
            ~SensorBody() { m_body->DestroyFixture(m_fixture); m_fixture = nullptr; }
            virtual const std::vector<std::vector<glm::vec2>>& getPolygonPoints() { return m_polygons; }
    };
}

#endif // SENSORBODY_H_INCLUDED
