#ifndef BOXBODY_H
#define BOXBODY_H

#include "phys/Body.h"

namespace CGameEngine
{
    class BoxBody : public Body
    {
        public:
            BoxBody(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, bool fixedRotation, float angle, int bodytype = BodyType::bodyDynamic, float density = 0.0f, float friction = 0.0f);
            ~BoxBody() { m_fixture = nullptr; }
            const std::vector<std::vector<glm::vec2>>& getPolygonPoints();
    };
}

#endif // BOXBODY_H
