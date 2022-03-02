#ifndef BODYDEBUGDRAW_H
#define BODYDEBUGDRAW_H

#include "Box2D/box2d.h"
#include "draw/GLSLProgram.h"

struct b2AABB;

namespace CGameEngine
{
	struct GLRenderPoints;
	struct GLRenderLines;
	struct GLRenderTriangles;

	struct DebugCamera
	{
		DebugCamera()
		{
			m_center.Set(0.0f, 0.0f);
			m_zoom = 0.2f;
			m_width = 1440;
			m_height = 900;
		}

		b2Vec2 ConvertScreenToWorld(const b2Vec2& screenPoint);
		b2Vec2 ConvertWorldToScreen(const b2Vec2& worldPoint);
		void BuildProjectionMatrix(float* m, float zBias);

		b2Vec2 m_center;
		float m_zoom;
		int32 m_width;
		int32 m_height;
	};

	class BodyDebugDraw : public b2Draw
	{
		public:
			BodyDebugDraw();
			virtual ~BodyDebugDraw();

			void Create();
			void Destroy();
			void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
			void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
			void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;
			void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;
			void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
			void DrawTransform(const b2Transform& xf) override;
			void DrawPoint(const b2Vec2& p, float size, const b2Color& color);
			//void DrawString(int x, int y, const char* string, ...);
			//void DrawString(const b2Vec2& p, const char* string, ...);
			void DrawAABB(b2AABB* aabb, const b2Color& color);
			void Flush();

		private:
			GLRenderPoints* m_points;
			GLRenderLines* m_lines;
			GLRenderTriangles* m_triangles;
	};
}

extern CGameEngine::BodyDebugDraw e_debugDraw;
extern CGameEngine::DebugCamera e_camera;

#endif // BODYDEBUGDRAW_H
