#ifndef BODY_H
#define BODY_H

#include "Box2D/box2d.h"
#include "common/types.h"
#include <unordered_map>
#include <vector>
#include "srv/ConfigReader.h"

namespace BodyType
{
    enum FORMS
    {
        bodyStatic = 0,
        bodyKinematic,
        bodyDynamic
    };
}

namespace BodyClass
{
    enum FORMS
    {
        noBody = 0,
        boxBody,		// 1
        capsuleBody,	// 2
        sphereBody,		// 3
        polygonBody,	// 4
        END
    };
}

namespace Sensors
{
    enum FORMS
    {
        NONE = 0,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left,
        TopLeft,
        END
    };
}

namespace CGameEngine
{
    class Body
    {
        /// (Body.type)
        /// static: zero mass, zero velocity, may be manually moved
        /// kinematic: zero mass, non-zero velocity set by user, moved by solver
        /// dynamic: positive mass, non-zero velocity determined by forces, moved by solver

        public:
            virtual void init(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, bool fixedRotation, float angle, int bodytype = BodyType::bodyDynamic, float density = 0.0f, float friction = 0.0f) {} // ctor
            virtual ~Body(); // dtor
            virtual const std::vector<std::vector<glm::vec2>>& getPolygonPoints() = 0;
            const std::vector<std::vector<glm::vec2>>& getRawPolygonPoints() { return m_polygons; }

            b2Body* getb2Body() const { return m_body; }
            bool hasContact(int sensor);
            bool hasBottomContact() { return hasContact(Sensors::Bottom); }
            bool hasTopContact() { return hasContact(Sensors::Top); }
            bool hasSideContact(bool left = true);
            const int getContacts() const;
            const b2Fixture* getFixture() const { return m_fixture; }
            const b2Vec2& getb2WorldCenter() const { return m_body->GetWorldCenter(); }
            const float getAngle() const { return m_body->GetAngle(); }
            const glm::vec2& getDimensions() const { return m_dimensions; }
            const glm::vec2 getPosition() const;
            const glm::vec4 getDestRect() const;
            const uint8_t getBodyClass() const { return m_bodyClass; }
            void* getUserData() { return ((m_body != nullptr) ? m_body->GetUserData() : nullptr); }

        protected:
            Body() {}
            b2Body* m_body = nullptr;
            b2Fixture* m_fixture = nullptr;
            std::unordered_map<int, b2Fixture*> m_sensors;
            std::vector<std::vector<glm::vec2>> m_polygons;
            glm::vec2 m_dimensions = glm::vec2(0.0f);
            uint8_t m_bodyClass = BodyClass::noBody;
    };
}

#endif // BODY_H
