#ifndef POLYGONBODY_H
#define POLYGONBODY_H

#include "phys/Body.h"

/*
	The position is the "center" for the polygons (0.0, 0.0). The points are added to the position value.
		e.g. position of (64.0, 64.0) with the point (0.3, 0.2) becomes (64.3, 64.2)
*/

const int MAXPOINTS = 8;

namespace CGameEngine
{
	class PolygonBody : public Body
	{
		public:
			PolygonBody(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, const std::vector<std::vector<glm::vec2>>& points, bool fixedRotation = true, float angle = 0.0f, int bodytype = BodyType::bodyDynamic, float density = 0.0f, float friction = 0.0f);
			PolygonBody(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, const std::string& json, bool fixedRotation = true, float angle = 0.0f, int bodytype = BodyType::bodyDynamic, float density = 0.0f, float friction = 0.0f);
			~PolygonBody() { }
			const std::vector<std::vector<glm::vec2>>& getPolygonPoints();

		private:
			void create(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, const std::vector<std::vector<glm::vec2>>& points, bool fixedRotation, float angle, int bodytype, float density, float friction);
			std::vector<std::vector<glm::vec2>> m_polygonPoints;
	};
}

#endif // POLYGONBODY_H
