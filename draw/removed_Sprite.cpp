#include "draw/Sprite.h"
#include "draw/Vertex.h"
#include "srv/ResourceManager.h"

namespace CGameEngine
{
    Sprite::~Sprite() { if(!m_vboID) { glDeleteBuffers(1, &m_vboID); } }

    void Sprite::draw()
    {
        glBindTexture(GL_TEXTURE_2D, m_texture.id); // actually bind texture in the sprite
        glBindBuffer(GL_ARRAY_BUFFER, m_vboID);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        // glVertexAttribPointer(array_id, size, type, normalized, stride, memory offset in byte)
        // size = 2 (x/y) or 3 (x/y/z)
        // type = datatype
        // normalized = convert from 0-255 to 0.0 - 1.0
        //glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0); // this is the position attribute point
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos)); // vertex
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color)); // color
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv)); // uv attribute
        // glDrawArrays(type, starting location, number of verticies)
        glDrawArrays(GL_TRIANGLES, 0, 6); // draw the 6 vertices to the screen
        glDisableVertexAttribArray(0); // disable the vertex attrib array (cleanup)
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind from the buffer id
    }

    void Sprite::init(float x, float y, float w, float h, std::string texturePath)
    {
        m_x = x;
        m_y = y;
        m_width = w;
        m_height = h;

        m_texture = ResourceManager::getTexture(texturePath);

        if(!m_vboID) { glGenBuffers(1, &m_vboID); } // generate vertex buffer object ID, if not exists

        // square is a quad, quad is always two triangles (quad because 4 points or verticies?)
        Vertex vertexData[6];

        // Tri 1:top right-corner
        vertexData[0].set(x + m_width, y + m_height, 1.0f, 1.0f);
        // Tri 1:top left
        vertexData[1].set(x, y + m_height, 0.0f, 1.0f);
        // Tri 1:bottom left
        vertexData[2].set(x, y, 0.0f, 0.0f);
        // Tri 2:top right-corner
        vertexData[3].set(x, y, 0.0f, 0.0f);
        // Tri 2:top left
        vertexData[4].set(x + m_width, y, 1.0f, 0.0f);
        // Tri 2:bottom left
        vertexData[5].set(x + m_width, y + m_height, 1.0f, 1.0f);

        for(int i = 0; i < 6; i++)
        {
            vertexData[i].setColor(255, 255, 255, 255);
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_vboID); // bind to our buffer ID
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW); // upload data to gpu
        glBindBuffer(GL_ARRAY_BUFFER, 0); // unbind from the buffer id
    }

    void copy(Sprite& dst, const Sprite& src)
    {
        if(&dst != &src)
        {
            dst.m_texture = src.m_texture;
            dst.m_vboID = src.m_vboID;
            dst.m_height = src.m_height;
            dst.m_width = src.m_width;
            dst.m_x = src.m_x;
            dst.m_y = src.m_y;
        }
    }

    void swap(Sprite& dst, Sprite& src)
    {
        if(&dst != &src)
        {
            dst.m_texture = src.m_texture;
            dst.m_vboID = src.m_vboID;
            dst.m_height = src.m_height;
            dst.m_width = src.m_width;
            dst.m_x = src.m_x;
            dst.m_y = src.m_y;
        }
    }
}
