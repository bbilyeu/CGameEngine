#include "draw/SpriteBatch.h"
//#include "common/util.h"
#include "common/glm_util.h"
#include "draw/GLSLProgram.h"

#include <algorithm>

namespace CGameEngine
{
    Glyph::Glyph(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint Texture, float Depth, const glm::ivec4& color) : texture(Texture), depth(Depth)
    {
        // top left
        topLeft.position = glm::vec2(destRect.x, destRect.y + destRect.w);
        topLeft.uv = glm::vec2(uvRect.x, uvRect.y + uvRect.w);
        topLeft.color = color;
        // bottom left
        bottomLeft.position = glm::vec2(destRect.x, destRect.y);
        bottomLeft.uv = glm::vec2(uvRect.x, uvRect.y);
        bottomLeft.color = color;
        // top right
        topRight.position = glm::vec2(destRect.x + destRect.z, destRect.y + destRect.w);
        topRight.uv = glm::vec2(uvRect.x + uvRect.z, uvRect.y + uvRect.w);
        topRight.color = color;
        // bottom right
        bottomRight.position = glm::vec2(destRect.x + destRect.z, destRect.y);
        bottomRight.uv = glm::vec2(uvRect.x + uvRect.z, uvRect.y);
        bottomRight.color = color;
    }

    Glyph::Glyph(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint Texture, float Depth, const glm::ivec4& color, float angle) : texture(Texture), depth(Depth)
    {
        glm::vec2 halfDimensions(destRect.z / 2.0f, destRect.w / 2.0f);

        // get points centered at origin
        glm::vec2 tl(-halfDimensions[0], halfDimensions[1]); // top left
        glm::vec2 bl(-halfDimensions[0], -halfDimensions[1]); // bottom left
        glm::vec2 tr(halfDimensions[0], halfDimensions[1]); // top right
        glm::vec2 br(halfDimensions[0], -halfDimensions[1]); // bottom right

        // rotate the points
        tl = rotatePoint(tl, angle) + halfDimensions;
        bl = rotatePoint(bl, angle) + halfDimensions;
        tr = rotatePoint(tr, angle) + halfDimensions;
        br = rotatePoint(br, angle) + halfDimensions;

        // top left
        topLeft.position = glm::vec2(destRect.x + tl.x, destRect.y + tl.y);
        topLeft.uv = glm::vec2(uvRect.x, uvRect.y + uvRect.w);
        topLeft.color = color;
        // bottom left
        bottomLeft.position = glm::vec2(destRect.x + bl.x, destRect.y + bl.y);
        bottomLeft.uv = glm::vec2(uvRect.x, uvRect.y);
        bottomLeft.color = color;
        // top right
        topRight.position = glm::vec2(destRect.x + tr.x, destRect.y + tr.y);
        topRight.uv = glm::vec2(uvRect.x + uvRect.z, uvRect.y + uvRect.w);
        topRight.color = color;
        // bottom right
        bottomRight.position = glm::vec2(destRect.x + br.x, destRect.y + br.y);
        bottomRight.uv = glm::vec2(uvRect.x + uvRect.z, uvRect.y);
        bottomRight.color = color;
    }

    void copy(Glyph& dst, const Glyph& src)
    {
        if(&dst != &src)
        {
            dst.texture = src.texture;
            dst.depth = src.depth;
            dst.topLeft = src.topLeft;
            dst.bottomLeft = src.bottomLeft;
            dst.topRight = src.topRight;
            dst.bottomRight = src.bottomRight;
        }
    }

    /// SpriteBatch public below //////////////////////////////////////////////

    SpriteBatch::SpriteBatch() {} // empty

    SpriteBatch::~SpriteBatch()
    {
		dispose();
    }

    void SpriteBatch::dispose()
    {
		if(m_vao) { glDeleteVertexArrays(1, &m_vao); }
        if(m_vbo) { glDeleteBuffers(1, &m_vbo); }

		m_glsl = nullptr;
        m_vbo = 0;
        m_vao = 0;
        m_glyphPointers.clear();
        m_glyphs.clear();
        m_renderBatches.clear();
        m_initialized = false;
        m_scale = 0.0f;
        m_screenHeight = 0.0f;
        m_screenWidth = 0.0f;
        m_sortType = GlyphSortType::TEXTURE;
    }

    void copy(SpriteBatch& dst, const SpriteBatch& src)
    {
        if(&dst != &src)
        {
            dst.m_glsl = src.m_glsl;
            dst.m_vbo = src.m_vbo;
            dst.m_vao = src.m_vao;
            dst.m_sortType = src.m_sortType;
            dst.m_glyphs = src.m_glyphs;
            dst.m_glyphPointers = src.m_glyphPointers;
            dst.m_renderBatches = src.m_renderBatches;
            dst.m_scale = src.m_scale;
            dst.m_screenHeight = src.m_screenHeight;
            dst.m_screenWidth = src.m_screenWidth;
        }
    }

    void SpriteBatch::begin(GlyphSortType sortType /*= GlyphSortType::TEXTURE*/) // all the pre-drawing work
    {
        m_sortType = sortType;
        // prevent buffer overflow by resizing vector to size of 0
        m_renderBatches.clear();
        m_glyphs.clear();
        m_glyphPointers.clear();
    }

    bool SpriteBatch::compareTexture(Glyph* a, Glyph* b)
    {
        return (a->texture < b->texture);
    }

    bool SpriteBatch::compareBackToFront(Glyph* a, Glyph* b)
    {
        return (a->depth > b->depth);
    }

    bool SpriteBatch::compareFrontToBack(Glyph* a, Glyph* b)
    {
        return (a->depth < b->depth);
    }

    // treating x=x, y=y, z=width, w=height
    void SpriteBatch::draw(const glm::vec4& destRect,const glm::vec4& uvRect, GLuint texture, float depth, const glm::ivec4& color) // add sprites to batch for rendering, perhaps rename?
    {
        m_glyphs.emplace_back(destRect, uvRect, texture, depth, color);
    }

    void SpriteBatch::draw(const glm::vec4& destRect,const glm::vec4& uvRect, GLuint texture, float depth, const glm::ivec4& color, float angle) // angle in radians
    {
        m_glyphs.emplace_back(destRect, uvRect, texture, depth, color, angle);
    }

    void SpriteBatch::draw(const glm::vec4& destRect,const glm::vec4& uvRect, GLuint texture, float depth, const glm::ivec4& color, const glm::vec2& direction) // direction
    {
        const glm::vec2 right(1.0f, 0.0f); // facing east or right
        float angle = acos(glm::dot(right, direction));
        if(direction.y < 0.0f) { angle = -angle; }

        m_glyphs.emplace_back(destRect, uvRect, texture, depth, color, angle);
    }

    void SpriteBatch::drawRepeat(const glm::vec4& glyphSize, const glm::vec4& uvRect, const glm::vec4& sizeBounds, GLuint texture, float depth, const glm::ivec4& color, float angle) // angle in radians
    {
        int x_iterations = static_cast<int>(sizeBounds.z / glyphSize.z);
        int y_iterations = static_cast<int>(sizeBounds.w / glyphSize.w);

        // tmp variable
        glm::vec4 tdr = glyphSize;
        glm::vec2 halfDimensions(sizeBounds.z / 2.0f, sizeBounds.w / 2.0f);

        // get points centered at origin
        glm::vec2 tl(-halfDimensions[0], halfDimensions[1]); // top left
        glm::vec2 bl(-halfDimensions[0], -halfDimensions[1]); // bottom left
        glm::vec2 tr(halfDimensions[0], halfDimensions[1]); // top right
        glm::vec2 br(halfDimensions[0], -halfDimensions[1]); // bottom right
        glm::vec2 positionOffset(sizeBounds.x, sizeBounds.y); // offset of x,y

        // rotate the points
        tl = rotatePoint(tl, angle) + halfDimensions + positionOffset;
        bl = rotatePoint(bl, angle) + halfDimensions + positionOffset;
        tr = rotatePoint(tr, angle) + halfDimensions + positionOffset;
        br = rotatePoint(br, angle) + halfDimensions + positionOffset;
        //std::cout<<"tl ["<<tl.x<<","<<tl.y<<"]\t\ttr ["<<tr.x<<","<<tr.y<<"]\n";
        //std::cout<<"bl ["<<bl.x<<","<<bl.y<<"]\t\tbr ["<<br.x<<","<<br.y<<"]\n";

        glm::vec2 blTObr((br.x - bl.x) / x_iterations, (br.y - bl.y) / y_iterations);
        glm::vec2 blTOtl((tl.x - bl.x) / x_iterations, (tl.y - bl.y) / y_iterations);

        // create glyphs
        for(unsigned int ix = 0; ix < x_iterations; ix++)
        {
            for(unsigned int iy = 0; iy < y_iterations; iy++)
            {
                tdr.x = bl.x + (blTObr.x * ix); // set X position with adjustements for movement from bottom left to bottom right
                tdr.y = bl.y + (blTObr.y * ix); // set Y position  with adjustements for movement from bottom left to bottom right
                tdr.x += blTOtl.x * iy; // offset x adjustments for movement from bottom left to top left
                tdr.y += blTOtl.y * iy; // offset y adjustments for movement from bottom left to top left

                if( (ix + iy) == 0 ) { m_glyphs.emplace_back(tdr, uvRect, texture, depth, COLOR_BLUE, angle); } // bottomLeft (blue)
                else if(ix == x_iterations-1 && iy == y_iterations-1) { m_glyphs.emplace_back(tdr, uvRect, texture, depth, COLOR_RED, angle); } // topRight (red)
                else { m_glyphs.emplace_back(tdr, uvRect, texture, depth, COLOR_GREEN, angle); } // all the rest (green)
            }
        }
    }

    void SpriteBatch::end() // post processing
    {
        // resize the glyphPointers vector to size of glyphs
        m_glyphPointers.resize(m_glyphs.size());
        // assign the actual addresses of the glyphs to the glyphPointers vectors
        for(int i = 0; i < m_glyphs.size(); i++) { m_glyphPointers[i] = &m_glyphs[i]; }
        sortGlyphs();
        createRenderBatches();
    }

    void SpriteBatch::init() // initialize with data
    {
		if(m_initialized) { return; }
        createVertexArray();
        m_initialized = true;
    }

    void SpriteBatch::renderBatch() // render to screen
    {
        glBindVertexArray(m_vao);
        //logger.Log(Logs::DEBUG, "SpriteBatch::renderBatch()", "[{}] draw calls", m_renderBatches.size());
        for(unsigned int i = 0; i < m_renderBatches.size(); i++)
        {
            glBindTexture(GL_TEXTURE_2D, m_renderBatches[i].texture);
            glDrawArrays(GL_TRIANGLES, m_renderBatches[i].offset, m_renderBatches[i].numVertices);
        }
        glBindVertexArray(0);
    }

    /// SpriteBatch private below /////////////////////////////////////////////

    void SpriteBatch::createRenderBatches()
    {
        if(m_glyphPointers.empty()) { return; } // no glyphs, move along

        std::vector<Vertex> vertices;
        int cv = 0; // current vertex
        int glyphOffset = 0;
        // allocate memory based on estimated size of vector
        vertices.resize(m_glyphPointers.size() * 6);
        // emplace_back uses the constructor for whatever is being stored
        //      in this case, it's RenderBatch
        m_renderBatches.emplace_back(glyphOffset, 6, m_glyphPointers[0]->texture);

        vertices[cv++] = m_glyphPointers[0]->topLeft; // execute, THEN add 1 to cv
        vertices[cv++] = m_glyphPointers[0]->bottomLeft; // execute, THEN add 1 to cv
        vertices[cv++] = m_glyphPointers[0]->bottomRight; // execute, THEN add 1 to cv
        vertices[cv++] = m_glyphPointers[0]->bottomRight; // execute, THEN add 1 to cv
        vertices[cv++] = m_glyphPointers[0]->topRight; // execute, THEN add 1 to cv
        vertices[cv++] = m_glyphPointers[0]->topLeft; // execute, THEN add 1 to cv
        glyphOffset += 6;

        for(unsigned int cg = 1; cg < m_glyphPointers.size(); cg++)
        {
            if(m_glyphPointers[cg]->texture != m_glyphPointers[cg - 1]->texture) { m_renderBatches.emplace_back(glyphOffset, 6, m_glyphPointers[cg]->texture); }
            else { m_renderBatches.back().numVertices += 6; }

            vertices[cv++] = m_glyphPointers[cg]->topLeft; // execute, THEN add 1 to cv
            vertices[cv++] = m_glyphPointers[cg]->bottomLeft; // execute, THEN add 1 to cv
            vertices[cv++] = m_glyphPointers[cg]->bottomRight; // execute, THEN add 1 to cv
            vertices[cv++] = m_glyphPointers[cg]->bottomRight; // execute, THEN add 1 to cv
            vertices[cv++] = m_glyphPointers[cg]->topRight; // execute, THEN add 1 to cv
            vertices[cv++] = m_glyphPointers[cg]->topLeft; // execute, THEN add 1 to cv
            glyphOffset += 6;
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        // vector.data() is same as vector[0]
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW); // orphan the buffer
        //glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

        // unbind buffer on completion
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void SpriteBatch::createVertexArray()
    {
        if(!m_vao) { glGenVertexArrays(1, &m_vao); } // generate vertex array
        glBindVertexArray(m_vao); // anything to change open gl state will be "read" and stored
        if(!m_vbo) { glGenBuffers(1, &m_vbo); } // generate vertex buffer
        // any time we rebind glBindVertexArray, it automatically calls this
        //      eliminating the need to do glBindBuffer after initial one
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position)); // vertex
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color)); // color
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv)); // uv
        //glVertexAttribDivisor(0, 1);
        //glVertexAttribDivisor(1, 1);
        //glVertexAttribDivisor(2, 1);

        glBindVertexArray(0); // free up vertex array
    }

    void SpriteBatch::sortGlyphs()
    {
        switch (m_sortType)
        {
            case GlyphSortType::BACK_TO_FRONT:
                std::stable_sort(m_glyphPointers.begin(), m_glyphPointers.end(), compareFrontToBack);
                break;
            case GlyphSortType::FRONT_TO_BACK:
                std::stable_sort(m_glyphPointers.begin(), m_glyphPointers.end(), compareBackToFront);
                break;
            case GlyphSortType::TEXTURE:
            default:
                std::stable_sort(m_glyphPointers.begin(), m_glyphPointers.end(), compareTexture);
                break;
        }
    }
}
