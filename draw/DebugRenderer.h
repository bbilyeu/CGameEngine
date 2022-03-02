#ifndef DEBUGRENDERER_H
#define DEBUGRENDERER_H

#include "draw/GLSLProgram.h"
#include "common/types.h"
#include <glm/glm.hpp>
#include <vector>

// draw outlines (or bounding-box wireframe) for debugging physics
namespace CGameEngine
{
	class Body;

    class DebugRenderer
    {
        public:
            DebugRenderer();
            ~DebugRenderer();

            void dispose(); // init's arch nemesis
            //void drawBody(CGameEngine::Body* body, const glm::ivec4& color);
            void drawBox(const glm::vec4 destRect, const glm::ivec4& color, float angle = 0.0f);
            void drawCircle(const glm::vec2& center, const glm::ivec4& color, float radius);
            void drawCoordinates(std::vector<glm::vec2> c, const glm::ivec4 color = COLOR_WHITE);
            void drawCoordinates(std::vector<std::vector<glm::vec2>> vc, const glm::ivec4 color = COLOR_WHITE);
            void drawLine(const glm::vec2& pointA, const glm::vec2& pointB, const glm::ivec4 color = COLOR_YELLOW);
            void drawPath(std::vector<glm::vec2> c, const glm::ivec4 color = COLOR_WHITE);
            void end(); // end() and renderBatch() combined
            void init();
            void render(float lineWidth);
            void render(const glm::mat4& projectionMatrix, float lineWidth);
            void render(const glm::mat4& projectionMatrix, const glm::vec2& newPosition, float lineWidth);

        private:
            CGameEngine::GLSLProgram m_shaderProgram;
            glm::vec2 rotatePoint(glm::vec2 pos, float angle);
            glm::vec3 rotatePoint(glm::vec3 pos, float angle);
            bool m_initialized = false;
            int m_numElements = 0;
            std::vector<DebugVertex> m_verts;
            std::vector<GLuint> m_indices;
            GLuint m_vbo = 0, m_vao = 0, m_ibo = 0; // vertex buffer object, vertex array object, index buffer object
            GLint m_uniformProjection = -1;
            const std::string uniformOne = "projection";
    };
}
#endif // DEBUGRENDERER_H
