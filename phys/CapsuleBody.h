#ifndef CAPSULEBODY_H
#define CAPSULEBODY_H

#include "phys/Body.h"
#include <vector>

namespace CGameEngine
{
    class CapsuleBody : public Body
    {
        // fixtures[0] = box (center mass)
        // fixtures[1] = circle (bottom/feet)
        // fixtures[2] = circle (top/head)

        public:
            CapsuleBody(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, bool fixedRotation, float angle, int bodytype = BodyType::bodyDynamic, float density = 0.0f, float friction = 0.0f, std::string json = "");
            ~CapsuleBody();
            const std::vector<std::vector<glm::vec2>>& getPolygonPoints();

            // build abstract layer for draw
            b2Fixture* getFixture(int index)  { return ((index >= 0 && index <= NUM_FIXTURES) ? m_fixtures[index] : nullptr); }

        private:

			struct LazyCircle
			{
				glm::vec2 offsetPos;
				float radius;
			};

            const int NUM_FIXTURES = 3;
            std::vector<b2Fixture*> m_fixtures;
            std::vector<LazyCircle> m_circles;
    };
}

#endif // CAPSULEBODY_H
