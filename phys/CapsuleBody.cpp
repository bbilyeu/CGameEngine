#include "phys/CapsuleBody.h"


namespace CGameEngine
{
    CapsuleBody::~CapsuleBody()
    {
        if(!m_fixtures.empty() && m_body)
        {
            for(unsigned int i = 0; i < m_fixtures.size(); i++)
            {
                m_body->DestroyFixture(m_fixtures[i]);
                m_fixtures[i] = nullptr;
            }
        }
    }

	CapsuleBody::CapsuleBody(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, bool fixedRotation, float angle, int bodytype /*= BodyType::bodyDynamic*/, float density /*= 0.0f*/, float friction /*= 0.0f*/, std::string json /*= ""*/)
    {
		if(!world) { return; }

		if(json != "")
		{
			ConfigReader* doc = new ConfigReader(json, false);
			if(doc && doc->isValid())
			{
				std::vector<std::string> members = doc->getMembers();
				if(members.size() == 1) // generic values (+/- y)
				{
					std::vector<float> data = doc->getArrayValue<float>(members[0]);
					if(data.size() == 3)
					{
						// top or bottom
						LazyCircle lz1 = { glm::vec2(data[0], data[1]), data[2] };
						m_circles.push_back(lz1);

						// inverted for other
						LazyCircle lz2 = { glm::vec2(data[0], (data[1] * -1.0f)), data[2] };
						m_circles.push_back(lz2);

					}
				}
				else if(members.size() == 2)
				{
					std::vector<float> data = doc->getArrayValue<float>(members[0]);
					if(data.size() == 3) { LazyCircle lc = { glm::vec2(data[0], data[1]), data[2] }; m_circles.push_back(lc); }
					data.clear();
					data = doc->getArrayValue<float>(members[1]);
					if(data.size() == 3) { LazyCircle lc = { glm::vec2(data[0], data[1]), data[2] }; m_circles.push_back(lc); }
				}
				else
				{
					Logger::getInstance().Log(Logs::CRIT, Logs::Physics, "CapsuleBody::CapsuleBody(json)", "members size was off! Expected 1 or 2, got [{}].", members.size());
				}
			}
		}

        // catch to prevent zero values
        if(bodytype == BodyType::bodyStatic) { density = 0.0f; friction = 0.0f; }
        else { density = (density <= 0.0f) ? 1.0f : density; } // set 1.0f density if it's 0.0f

        m_dimensions = dimensions;
        b2BodyDef bodyDef; // create body
        bodyDef.type = (b2BodyType)bodytype;
        bodyDef.position.Set(position.x, position.y); // set position
        bodyDef.fixedRotation = fixedRotation; // if true, no rotation
        m_body = world->CreateBody(&bodyDef); // creates the body, need shape and then fixture

        // create box (center)
        b2PolygonShape boxShape; // create shape
        //boxShape.SetAsBox(m_dimensions.x / 2.0f, (m_dimensions.y - m_dimensions.x) / 2.0f); // specify shape
        boxShape.SetAsBox(m_dimensions.x / 2.0f, m_dimensions.y / 2.0f); // specify shape

        // fixture to the box
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &boxShape;
        fixtureDef.density = density * 3; // was 3.0f ; dynamic (moving) objects MUST have a non-zero density!!!
        fixtureDef.friction = friction * 0.66f; // was 0.2f
        m_fixtures.push_back(m_body->CreateFixture(&fixtureDef)); // fixture binds body to fixtureDefinition

        // create the circle
        b2CircleShape circleShape;
        circleShape.m_radius = m_dimensions.x / 2.0f;

        // fixture to the circle
        b2FixtureDef circleDef;
        circleDef.shape = &circleShape;
        circleDef.density = density; // was 1.0f
        circleDef.friction = friction; // was 0.3f
        m_bodyClass = BodyClass::capsuleBody;

        // store box coordinates
        m_polygons.resize(3);
        m_polygons[0].push_back(glm::vec2(position.x, position.y));
        m_polygons[0].push_back(glm::vec2(position.x + dimensions.x, position.y));
        m_polygons[0].push_back(glm::vec2(position.x + dimensions.x, position.y + dimensions.y));
        m_polygons[0].push_back(glm::vec2(position.x, position.y + dimensions.y));

		// populate circles vector (if needed)
		if(m_circles.size() == 0)
		{
			//LazyCircle lzBottom = { glm::vec2(0.0f, (-m_dimensions.y + +m_dimensions.x) / 2.0f), (m_dimensions.x / 2.0f) };
			LazyCircle lzBottom = { glm::vec2(0.0f, (-m_dimensions.y / 2.0f)), (m_dimensions.x / 2.0f) };
			m_circles.push_back(lzBottom);
			//LazyCircle lzTop = { glm::vec2(0.0f, (+m_dimensions.y + -m_dimensions.x) / 2.0f), (m_dimensions.x / 2.0f) };
			LazyCircle lzTop = { glm::vec2(0.0f, (+m_dimensions.y / 2.0f)), (m_dimensions.x / 2.0f) };
			m_circles.push_back(lzTop);
		}

        // bottom circle
        circleShape.m_radius = m_circles[0].radius;
        circleShape.m_p.Set(m_circles[0].offsetPos.x, m_circles[0].offsetPos.y); // offset from center of body
        b2Fixture* bottomCircle = m_body->CreateFixture(&circleDef);
        if(bottomCircle) { m_fixtures.push_back(bottomCircle); }
        else { Logger::getInstance().Log(Logs::CRIT, Logs::Physics, "CapsuleBody::CapsuleBody()", "Failed to create bottom circle!"); return; };
        m_polygons[1].push_back(glm::vec2(position.x + m_circles[0].offsetPos.x + (m_dimensions.x / 2.0f), position.y + (m_dimensions.y / 2.0f) + m_circles[0].offsetPos.y));
        m_polygons[1].push_back(glm::vec2(m_circles[0].radius));

        // top circle
        circleShape.m_radius = m_circles[1].radius;
        circleShape.m_p.Set(m_circles[1].offsetPos.x, m_circles[1].offsetPos.y); // offset from center of body
        b2Fixture* topCircle = m_body->CreateFixture(&circleDef);
        if(topCircle) { m_fixtures.push_back(topCircle); }
        else { Logger::getInstance().Log(Logs::CRIT, Logs::Physics, "CapsuleBody::CapsuleBody()", "Failed to create top circle!"); return; };
        m_polygons[2].push_back(glm::vec2(position.x + m_circles[1].offsetPos.x + (m_dimensions.x / 2.0f), position.y + (m_dimensions.y / 2.0f) + m_circles[1].offsetPos.y));
        m_polygons[2].push_back(glm::vec2(m_circles[1].radius));


        /// SENSOR DEFINITIONS
        /*fixtureDef.isSensor = true;
        char* val = new char('f');

        // top
        boxShape.SetAsBox(m_dimensions.x * 0.5f * 0.3f, m_dimensions.y * 0.5f * 0.3f, b2Vec2(0.0f, +m_dimensions.y + -m_dimensions.x), 0);
        m_sensors[Sensors::Top] = m_body->CreateFixture(&fixtureDef);
        m_sensors[Sensors::Top]->SetUserData( static_cast<void*>(val) );

        // top right
        boxShape.SetAsBox(m_dimensions.x * 0.5f * 0.3f, m_dimensions.y * 0.5f * 0.3f, b2Vec2(m_dimensions.x, +m_dimensions.y + -m_dimensions.x), 0);
        m_sensors[Sensors::TopRight] = m_body->CreateFixture(&fixtureDef);
        val = new char('f');
        m_sensors[Sensors::TopRight]->SetUserData( static_cast<void*>(val) );

        // right
        boxShape.SetAsBox(m_dimensions.x * 0.5f * 0.3f, m_dimensions.y * 0.5f * 0.3f, b2Vec2(m_dimensions.x, 0.0f), 0);
        m_sensors[Sensors::Right] = m_body->CreateFixture(&fixtureDef);
        val = new char('f');
        m_sensors[Sensors::Right]->SetUserData( static_cast<void*>(val) );

        // bottom right
        boxShape.SetAsBox(m_dimensions.x * 0.5f * 0.3f, m_dimensions.y * 0.5f * 0.3f, b2Vec2(m_dimensions.x, -m_dimensions.y + +m_dimensions.x), 0);
        m_sensors[Sensors::BottomRight] = m_body->CreateFixture(&fixtureDef);
        val = new char('f');
        m_sensors[Sensors::BottomRight]->SetUserData( static_cast<void*>(val) );

        // bottom
        boxShape.SetAsBox(m_dimensions.x * 0.5f * 0.3f, m_dimensions.y * 0.5f * 0.3f, b2Vec2(0.0f, -m_dimensions.y + m_dimensions.x), 0);
        m_sensors[Sensors::Bottom] = m_body->CreateFixture(&fixtureDef);
        val = new char('f');
        m_sensors[Sensors::Bottom]->SetUserData( static_cast<void*>(val) );

        // bottom left
        boxShape.SetAsBox(m_dimensions.x * 0.5f * 0.3f, m_dimensions.y * 0.5f * 0.3f, b2Vec2(-m_dimensions.x, -m_dimensions.y + +m_dimensions.x), 0);
        m_sensors[Sensors::BottomLeft] = m_body->CreateFixture(&fixtureDef);
        val = new char('f');
        m_sensors[Sensors::BottomLeft]->SetUserData( static_cast<void*>(val) );

        // left
        boxShape.SetAsBox(m_dimensions.x * 0.5f * 0.3f, m_dimensions.y * 0.5f * 0.3f, b2Vec2(-m_dimensions.x, 0.0f), 0);
        m_sensors[Sensors::Left] = m_body->CreateFixture(&fixtureDef);
        val = new char('f');
        m_sensors[Sensors::Left]->SetUserData( static_cast<void*>(val) );

        // top left
        boxShape.SetAsBox(m_dimensions.x * 0.5f * 0.3f, m_dimensions.y * 0.5f * 0.3f, b2Vec2(-m_dimensions.x, +m_dimensions.y + -m_dimensions.x), 0);
        m_sensors[Sensors::TopLeft] = m_body->CreateFixture(&fixtureDef);
        val = new char('f');
        m_sensors[Sensors::TopLeft]->SetUserData( static_cast<void*>(val) );*/
    }

	const std::vector<std::vector<glm::vec2>>& CapsuleBody::getPolygonPoints()
    {
		glm::vec2 position = getPosition();
		m_polygons[0][0] = position;
        m_polygons[0][1] = glm::vec2(position.x + m_dimensions.x, position.y);
        m_polygons[0][2] = glm::vec2(position.x + m_dimensions.x, position.y + m_dimensions.y);
        m_polygons[0][3] = glm::vec2(position.x, position.y + m_dimensions.y);
		m_polygons[1][0] = glm::vec2( (position.x + m_circles[0].offsetPos.x + (m_dimensions.x / 2.0f)) , (position.y + (m_dimensions.y / 2.0f) + m_circles[0].offsetPos.y) );
		m_polygons[2][0] = glm::vec2( (position.x + m_circles[1].offsetPos.x + (m_dimensions.x / 2.0f)) , (position.y + (m_dimensions.y / 2.0f) + m_circles[1].offsetPos.y) );
		return m_polygons;
	}
}
