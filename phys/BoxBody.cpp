#include "BoxBody.h"

namespace CGameEngine
{
    BoxBody::BoxBody(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, bool fixedRotation, float angle, int bodytype /*= BodyType::bodyDynamic*/, float density /*= 0.0f*/, float friction /*= 0.0f*/)
    {
        /// (Body.type)
        /// static: zero mass, zero velocity, may be manually moved
        /// kinematic: zero mass, non-zero velocity set by user, moved by solver
        /// dynamic: positive mass, non-zero velocity determined by forces, moved by solver

        if(!world) { return; }

        // catch to prevent zero values
        if(bodytype == BodyType::bodyStatic) { density = 0.0f; friction = 0.0f; }
        else { density = (density <= 0.0f) ? 1.0f : density; } // set 1.0f density if it's 0.0f

        m_dimensions = dimensions;
        b2BodyDef bodyDef; // create body
        bodyDef.type = (b2BodyType)bodytype;
        bodyDef.position.Set(position.x, position. y); // set position
        bodyDef.fixedRotation = fixedRotation; // if true, no rotation
        if(fixedRotation) { bodyDef.angle = angle; }
        m_body = world->CreateBody(&bodyDef); // creates the body, need shape and then fixture

        // create box (center)
        b2PolygonShape boxShape; // create shape
        //boxShape.SetAsBox(m_dimensions.x / 2.0f, (m_dimensions.y - m_dimensions.x) / 2.0f); // specify shape
        boxShape.SetAsBox(m_dimensions.x / 2.0f, m_dimensions.y / 2.0f); // specify shape

        // fixture to the box
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &boxShape;
        fixtureDef.density = density;
        fixtureDef.friction = friction;
        m_fixture = m_body->CreateFixture(&fixtureDef);
        m_bodyClass = BodyClass::boxBody;

        m_polygons.resize(1);
        m_polygons[0].push_back(glm::vec2(position.x, position.y));
        m_polygons[0].push_back(glm::vec2(position.x + m_dimensions.x, position.y));
        m_polygons[0].push_back(glm::vec2(position.x + m_dimensions.x, position.y + m_dimensions.y));
        m_polygons[0].push_back(glm::vec2(position.x, position.y + m_dimensions.y));
    }

	const std::vector<std::vector<glm::vec2>>& BoxBody::getPolygonPoints()
    {
		glm::vec2 position = getPosition();
		glm::vec2 offset = glm::vec2(m_dimensions.x / 2.0f, m_dimensions.y / 2.0f);
		m_polygons[0][0] = position;
        m_polygons[0][1] = glm::vec2(position.x + m_dimensions.x, position.y);
        m_polygons[0][2] = glm::vec2(position.x + m_dimensions.x, position.y + m_dimensions.y);
        m_polygons[0][3] = glm::vec2(position.x, position.y + m_dimensions.y);
		return m_polygons;
	}
}
