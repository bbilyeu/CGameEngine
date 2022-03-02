#ifndef SPHEREBODY_H
#define SPHEREBODY_H

#include "phys/Body.h"

namespace CGameEngine
{
    class SphereBody : public Body
    {
        public:
            SphereBody(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, bool fixedRotation, float angle, int bodytype = BodyType::bodyDynamic, float density = 0.0f, float friction = 0.0f);
            ~SphereBody() { m_body->DestroyFixture(m_fixture); m_fixture = nullptr; }
            const std::vector<std::vector<glm::vec2>>& getPolygonPoints();
    };
}

#endif // SPHEREBODY_H
