#ifndef SPRITEBATCH_H
#define SPRITEBATCH_H
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // orthographic view
#include <vector>
#include "common/types.h"

namespace CGameEngine
{

    class GLSLProgram;
    struct Glyph;
    struct GlyphGroup;

    struct RenderBatch
    {
        RenderBatch(GLuint OffSet, GLuint NumVertices, GLuint Texture) : offset(OffSet), numVertices(NumVertices), texture(Texture) { }
        GLuint offset = 0;
        GLuint numVertices = 0;
        GLuint texture = 0;
    };

    enum class GlyphSortType
    {
        NONE,           // naaah
        FRONT_TO_BACK,  // starting from front?
        BACK_TO_FRONT,  // starting from back?
        TEXTURE         // draw same textures
    };

    struct Glyph // single sprite, 6 vertices with what-type-of-texture and depth
    {
            Glyph() { }
            friend void copy(Glyph& dst, const Glyph& src);
            Glyph(const Glyph& g) { copy(*this, g); }
            Glyph& operator=(const Glyph& g) { copy(*this, g); return *this; }
            Glyph(const glm::vec4& destRect,const glm::vec4& uvRect, GLuint Texture, float Depth, const glm::ivec4& color);
            Glyph(const glm::vec4& destRect,const glm::vec4& uvRect, GLuint Texture, float Depth, const glm::ivec4& color, float angle); // angle in radians
            GLuint texture = 0;
            float depth = 0.0f;
            Vertex topLeft;
            Vertex bottomLeft;
            Vertex topRight;
            Vertex bottomRight;
    };

    // aim is to draw all 'same-type' textures at once, before drawing next ones
    //      i.e. all apple textures, then orange textures, etc
    class SpriteBatch
    {
        // treating x=x, y=y, z=width, w=height
        public:
            SpriteBatch();
            ~SpriteBatch();
            SpriteBatch(const SpriteBatch& sb) { copy(*this, sb); }
            SpriteBatch& operator=(const SpriteBatch& sb) { copy(*this, sb); return *this; }
            friend void copy(SpriteBatch& dst, const SpriteBatch& src);
            void begin(GlyphSortType sortType = GlyphSortType::TEXTURE); // all the pre-drawing work
            static bool compareBackToFront(Glyph* a, Glyph* b);
            static bool compareFrontToBack(Glyph* a, Glyph* b);
            static bool compareTexture(Glyph* a, Glyph* b);
            void draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const glm::ivec4& color);
            void draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const glm::ivec4& color, float angle); // angle in radians
            void draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const glm::ivec4& color, const glm::vec2& direction); // direction
            void drawRepeat(const glm::vec4& glyphSize, const glm::vec4& uvRect, const glm::vec4& sizeBounds, GLuint texture, float depth, const glm::ivec4& color, float angle); // angle in radians
            void end(); // post processing
            void init(); // initialize with data
            void renderBatch(); // render to screen
            void setScreenDimensions(float width, float height, float scale) { m_screenHeight = height; m_screenWidth = width; m_scale = scale; }
            void setGLSLProgram(GLSLProgram& glsl) { m_glsl = &glsl; }
            void dispose();

        private:
            GLSLProgram* m_glsl = nullptr;
            GLuint m_vbo = 0; // vertex buffer object
            GLuint m_vao = 0; // vertex array object
            GlyphSortType m_sortType = GlyphSortType::TEXTURE;
            std::vector<Glyph> m_glyphs; // actual glyphs
            std::vector<Glyph*> m_glyphPointers; // sorting only
            std::vector<RenderBatch> m_renderBatches;
            //void createGroupRenderBatches();
            void createRenderBatches();
            void createVertexArray();
            void sortGlyphs();
            bool m_initialized = false;
            float m_scale = 0.0f;
            float m_screenHeight = 0.0f;
            float m_screenWidth = 0.0f;
    };
}
#endif // SPRITEBATCH_H
