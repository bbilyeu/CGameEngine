#ifndef DRAW_COMMON_INCLUDED
#define DRAW_COMMON_INCLUDED

#include "common/types.h" // glm, Logger, GL* types
#include <vector>

#define QUAD_VERTS 4
#define TRIANGLE_VERTS 3

namespace CGameEngine
{
	class Texture2D;

	/**
	 * @brief stores the base values of the vertices, centered on the position
	 */
	struct Glyph
	{
		TextureAddress textureAddress; /**< container handle and slice number */
		std::vector<Vertex> vertices; /**< vector of the vertices (pos, uv) to draw */
		glm::vec3 position; /**< x/y/z coordinates */
		float angle; /**< rotation in radians */

		/**
		 * @brief compare two glyphs for sameness (A > B)
		 *
		 * @param other glyph to compare against
		 * @return true same glyph
		 * @return false different glyphs
		 */
		inline bool operator<(const Glyph& other)
		{
			if(this->textureAddress < other.textureAddress || this->vertices.size() < other.vertices.size()) { return true; }
			else if(this->textureAddress > other.textureAddress || this->vertices.size() > other.vertices.size()) { return false; }

			// if no return yet, compare each vertex
			for(size_t i = 0; i < other.vertices.size(); i++) { if(this->vertices[i] < other.vertices[i]) { return true; } }

			// if nothing at all was 'less than'
			return false;
		}

		/**
		 * @brief compare two glyphs for sameness (B < A)
		 *
		 * @param other
		 * @return true
		 * @return false
		 */
		inline bool operator>(const Glyph& other) { return !(*this < other); }
	};

	/**
	 * @brief struct for drawArraysIndirect calls
	 */
	struct DrawArraysIndirectCommand
	{
		GLuint vertexCount;		/**< vertices */
		GLuint instanceCount;	/**< number of instances to draw */
		GLuint firstVertex;		/**< start from which vertex, in the first index */
		GLuint baseInstance;	/**< inactive unless ARB_shader_draw_parameters is used (or glAttribPointer) */
	};
	typedef DrawArraysIndirectCommand DAIC;

	/**
	 * @brief struct for drawElementsIndirect calls
	 */
	struct DrawElementsIndirectCommand
	{
		GLuint vertexCount;		/**< vertices */
		GLuint instanceCount;	/**< number of instances to draw */
		GLuint firstIndex;		/**< first index in the element array buffer relative to current vertex */
		GLuint baseVertex;		/**< start from which vertex, in the first index */
		GLuint baseInstance;	/**< inactive unless ARB_shader_draw_parameters is used (or glAttribPointer) */
	};
	typedef DrawElementsIndirectCommand DEIC;

	/**
	 * @brief values for tracking position in a ring buffer
	 *
	 * @tparam T data type for the ring buffer (e.g. glm::vec2, DAIC, DEIC, etc)
	 */
	template<class T>
	struct RingBufferHead
	{
		int oldHead;
		int head;
		int bufferSize; // total size
		int bufferFragment; // 1/3 of total size
		T ptr;
	};

	/**
	 * @brief glyhp for a single character (font drawing)
	 */
	struct CharGlyph
	{
		char character = 0;
		glm::vec4 uvRect = glm::vec4(0.0f);
		glm::vec2 dimensions = glm::vec2(0.0f); // size
    };

	/**
	 * @brief stores data related to a font used in drawing (a.k.a font as images)
	 */
    struct ActiveFont
    {
        //Texture2D* texture = nullptr;
        TextureAddress textureAddress = {0,0};
        std::vector<CharGlyph*> cGlyphs;
        int startOffset = 0; // usually FIRST_PRINTABLE_CHAR
        int length = 0; // usually LAST_PRINTABLE_CHAR - FIRST_PRINTABLE_CHAR
        int fontHeight = 0;
        int fontSize = 0; // ex: 14pt

        glm::vec2 measure(const char* s, float scaling = 1.0f) // measures the dimensions of the text
        {
			glm::vec2 dimensions;
			dimensions.y = (float)(this->fontHeight * scaling);
			float charWidth = 0;
			for (int stringIndex = 0; s[stringIndex] != 0; stringIndex++)
			{
				char c = s[stringIndex];
				if (s[stringIndex] == '\n')
				{
					dimensions.y += fontHeight * scaling;
					if (dimensions.x < charWidth) { dimensions.x = charWidth; }
					charWidth = 0;
				}
				else
				{
					// Check for correct glyph
					int glyphIndex = c - startOffset;
					if (glyphIndex < 0 || glyphIndex >= length) { glyphIndex = length; }
					charWidth += cGlyphs[glyphIndex]->dimensions.x * scaling;
				}
			}
			if (dimensions.x < charWidth) { dimensions.x = charWidth; }

			return dimensions;
        }
        glm::vec2 measure(const std::string& s, float scaling = 1.0f) { return measure(s.c_str(), scaling); }
    };
}
#endif // DRAW_COMMON_INCLUDED
