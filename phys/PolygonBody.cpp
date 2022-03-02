#include "PolygonBody.h"
#include "srv/ConfigReader.h"

namespace CGameEngine
{
	PolygonBody::PolygonBody(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, const std::vector<std::vector<glm::vec2>>& points, bool fixedRotation /*= true*/, float angle /*= 0.0f*/, int bodytype /*= BodyType::bodyDynamic*/, float density /*= 0.0f*/, float friction /*= 0.0f*/)
	{
		create(world, position, dimensions, points, fixedRotation, angle, bodytype, density, friction);
	}

	/// \TODO: Add parse error try-catch
	PolygonBody::PolygonBody(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, const std::string& json, bool fixedRotation /*= true*/, float angle /*= 0.0f*/, int bodytype /*= BodyType::bodyDynamic*/, float density /*= 0.0f*/, float friction /*= 0.0f*/)
	{
		ConfigReader* doc = new ConfigReader(json, false);
		if(doc && doc->isValid())
		{
			std::vector<std::string> members = doc->getMembers();
			std::vector<std::vector<glm::vec2>> points;
			int polygonCount = members.size();
			points.resize(polygonCount);

			// get points per polygon
			for(unsigned int p = 0; p < polygonCount; p++) { points[p] = doc->getVec2Array(members[p]);	}

			// call actual creation function
			create(world, position, dimensions, points, fixedRotation, angle, bodytype, density, friction);
		}
	}

	/// PolygonBody private functions /////////////////////////////////////////

	void PolygonBody::create(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, const std::vector<std::vector<glm::vec2>>& points, bool fixedRotation, float angle, int bodytype, float density, float friction)
	{
		/// (Body.type)
        /// static: zero mass, zero velocity, may be manually moved
        /// kinematic: zero mass, non-zero velocity set by user, moved by solver
        /// dynamic: positive mass, non-zero velocity determined by forces, moved by solver

        if(!world) { return; }

        // catch to prevent zero values
        if(bodytype == BodyType::bodyStatic) { density = 0.0f; friction = 0.0f; }
        else { density = (density <= 0.0f) ? 1.0f : density; } // set 1.0f density if it's 0.0f

		b2BodyDef bodyDef; // create body
        bodyDef.type = (b2BodyType)bodytype;
        bodyDef.position.Set(position.x, position.y); // set position
        bodyDef.fixedRotation = fixedRotation; // if true, no rotation
        if(fixedRotation) { bodyDef.angle = angle; }
        m_body = world->CreateBody(&bodyDef); // creates the body, need shape and then fixture
        m_dimensions = dimensions;

        // reusable fixture definition
        b2FixtureDef fixtureDef;
        fixtureDef.density = density * 3;
        fixtureDef.friction = friction * 0.66f;
        m_bodyClass = BodyClass::polygonBody;

        // expand vector
        m_polygons.resize(points.size());

        // create polygons
        for(unsigned int i = 0; i < points.size(); i++) // points[x] (number of shapes)
        {
			unsigned int numPoints = points[i].size();
			if(numPoints > 0 && numPoints <= MAXPOINTS) // 1-8 points
			{
				m_polygons[i].resize(numPoints); // resize nested vector (points for current polygon)
				b2PolygonShape pshape;
				b2Vec2 vertices[MAXPOINTS]; // 8 max
				for(unsigned int j = 0; j < numPoints; j++) // points[x][y] (number of points in a shape)
				{
					vertices[j].Set( ((points[i][j].x * m_dimensions.x) + position.x), ((points[i][j].y * m_dimensions.y) + position.y) );
					m_polygons[i][j] = glm::vec2((points[i][j].x * m_dimensions.x), (points[i][j].y * m_dimensions.y)); // store polygon point
					//m_polygons[i][j] = glm::vec2( ((points[i][j].x * m_dimensions.x) + position.x), ((points[i][j].y * m_dimensions.y) + position.y) );
				}
				pshape.Set(vertices, numPoints); // pass points to shape
				fixtureDef.shape = &pshape;
				m_body->CreateFixture(&fixtureDef);
			}
        }

        // should be done by now
	}

	const std::vector<std::vector<glm::vec2>>& PolygonBody::getPolygonPoints()
    {
		m_polygonPoints = m_polygons;
		glm::vec2 pos = getPosition();
		glm::vec2 offset = pos - m_polygonPoints[0][0];
		glm::vec2 offsetInv = m_polygonPoints[0][0] - pos;
		Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "PolygonBody::getPolygonPoints()", "Origin ({}, {}), b2->GetPosition ({}, {}), offset ({}, {}), offset inv ({}, {})", m_polygonPoints[0][0].x, m_polygonPoints[0][0].y, pos.x, pos.y, offset.x, offset.y, offsetInv.x, offsetInv.y);
		for(unsigned int i = 0; i < m_polygonPoints.size(); i++)
		{
			for(unsigned h = 0; h < m_polygonPoints[i].size(); h++)
			{
				m_polygonPoints[i][h] += pos + offset;
			}
		}
		return m_polygonPoints;
	}
}
