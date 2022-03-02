#ifndef SPRITE_H
#define SPRITE_H

#define GL_GLEXT_PROTOTYPES
//#include <GLES3/gl3.h>
//#include <GL/gl.h>
#include <GL/glew.h>
#include <cstddef>
#include "draw/GLTexture.h"
#include <string>

namespace CGameEngine
{
    class Sprite
    {
        public:
            Sprite() {}
            ~Sprite();
            void draw();
            void init(float x, float y, float w, float h, std::string texturePath);
            Sprite(const Sprite& s) : Sprite() { copy(*this, s); } // copy constructor
            Sprite(Sprite&& s) : Sprite()  { swap(*this, s); } // move constructor
            Sprite& operator=(Sprite s) { swap(*this, s); return *this; } // copy/move assignment
            friend void copy(Sprite& dst, const Sprite& src);
            friend void swap(Sprite& dst, Sprite& src);

        private:
            GLTexture m_texture;
            GLuint m_vboID = 0;

            float m_height = 0.0f;
            float m_width = 0.0f;
            float m_x = 0.0f;
            float m_y = 0.0f;
    };
}
#endif // SPRITE_H
