//#ifndef VERTEX_H_INCLUDED
//#define VERTEX_H_INCLUDED
#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

// keep in multiples of 4 for alignment issues

// when storing a struct/class inside of another struct/class,
//  it is called 'composition'.

namespace CGameEngine
{
    /*struct Position // two floats, 4 bytes each
    {
        Position(float pX, float pY) : x(pX), y(pY) {}
        float x = 0.0f;
        float y = 0.0f;
    };*/

    struct ColorRGBA8  // four char, 1 byte each
    {
        ColorRGBA8() : r(0), g(0), b(0), a(0) { }
        ColorRGBA8(GLubyte R, GLubyte G, GLubyte B, GLubyte A) : r(R), g(G), b(B), a(A) { }
        GLubyte r = 0; // red
        GLubyte g = 0; // green
        GLubyte b = 0; // blue
        GLubyte a = 0; // alpha (transparency)

        ColorRGBA8(const ColorRGBA8& c) : ColorRGBA8() { copy(*this, c); } // copy constructor
        ColorRGBA8(ColorRGBA8&& c) : ColorRGBA8()  { swap(*this, c); } // move constructor
        ColorRGBA8& operator=(ColorRGBA8 c) { swap(*this, c); return *this; } // copy/move assignment
        friend void copy(ColorRGBA8& dst, const ColorRGBA8& src)
        {
            if(&dst != &src)
            {
                dst.r = src.r;
                dst.g = src.g;
                dst.b = src.b;
                dst.a = src.a;
            }
        }
        friend void swap(ColorRGBA8& dst, ColorRGBA8& src)
        {
            if(&dst != &src)
            {
                dst.r = src.r;
                dst.g = src.g;
                dst.b = src.b;
                dst.a = src.a;
            }
        }

        inline bool operator !=(const ColorRGBA8& clr) const
        {
            if(clr.r != r || clr.g != g || clr.b != g || clr.a != a) { return true; }
            else { return false; }
        }

        inline bool operator ==(const ColorRGBA8& clr) const
        {
            if(clr.r != r || clr.g != g || clr.b != g || clr.a != a) { return false; }
            else { return true; }
        }

        inline bool operator !() const
        {
            if(r == 0 && g == 0 && b == 0 && a == 0) { return true; }
            else { return false; }
        }

        GLubyte& operator[](size_t n)
        {
            switch(n)
            {
                case 0:
                    return r;
                case 1:
                    return g;
                case 2:
                    return b;
                case 3:
                    return a;
            }
        }

        const GLubyte& operator[](size_t n) const
        {
            switch(n)
            {
                case 0:
                    return r;
                case 1:
                    return g;
                case 2:
                    return b;
                case 3:
                    return a;
            }
        }

        /*inline bool operator ()() const
        {
            if(r != 0 || g != 0 || b != 0 || a != 0) { return true; }
            else { return false; }
        }*/
    };

    static ColorRGBA8 COLOR_NONE(0, 0, 0, 0);
    static ColorRGBA8 COLOR_BLACK(0, 0, 0, 255);
    static ColorRGBA8 COLOR_WHITE(255, 255, 255, 255);
    static ColorRGBA8 COLOR_GREY(163, 171, 183, 255);
    static ColorRGBA8 COLOR_RED(255, 0, 0, 255);
    static ColorRGBA8 COLOR_GREEN(0, 255, 0, 255);
    static ColorRGBA8 COLOR_BLUE(0, 0, 255, 255);
    static ColorRGBA8 COLOR_YELLOW(255, 255, 0, 255);
    static ColorRGBA8 COLOR_PURPLE(128, 0, 187, 255);
    static ColorRGBA8 COLOR_ORANGE(255, 128, 0, 255);
    static ColorRGBA8 COLOR_TRANSPARENT(255, 255, 255, 0);

    struct UV
    {
        UV(float pU, float pV) : u(pU), v(pV) {}
        float u = 0.0f;
        float v = 0.0f;
    };

    // adding the variable name after the struct definition saves the "Color color;" definition
    struct Vertex
    {
        Vertex() {}
        ColorRGBA8 color = COLOR_WHITE; // 4 bytes for R G B A
        glm::vec4 pos = glm::vec4(0.0f);
        glm::vec4 uv = glm::vec4(0.0f); // UV texture coordinates

        Vertex(const Vertex& v) { copy(*this, v); } // copy constructor
        Vertex(Vertex&& v) { swap(*this, v); } // move constructor
        Vertex& operator=(Vertex v) { swap(*this, v); return *this; } // copy/move assignment
        friend void copy(Vertex& dst, const Vertex& src)
        {
            if(&dst != &src)
            {
                dst.pos = src.pos;
                dst.uv = src.uv;
                dst.color = src.color;
            }
        }
        friend void swap(Vertex& dst, Vertex& src)
        {
            if(&dst != &src)
            {
                dst.pos = src.pos;
                dst.uv = src.uv;
                dst.color = src.color;
            }
        }

        void set(float x, float y, float u, float v)
        {
            pos.x = x; pos.y = y;
            uv[0] = u; uv[1] = v;
        }

        void setPosition(float x, float y)
        {
            pos.x = x;
            pos.y = y;
        }

        void shiftPosition(float x, float y)
        {
            pos.x += x;
            pos.y += y;
        }

        void setColor(GLubyte R, GLubyte G, GLubyte B, GLubyte A)
        {
            color.r = R;
            color.g = G;
            color.b = B;
            color.a = A;
        }

        void setUV(float u, float v)
        {
            uv[0] = u;
            uv[1] = v;
        }
    };
}
//#endif // VERTEX_H_INCLUDED
