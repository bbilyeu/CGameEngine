#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#define FIRST_PRINTABLE_CHAR ((char)32)
#define LAST_PRINTABLE_CHAR ((char)126)

#include "common/types.h"
#include "draw/draw_common.h"
#include <map>

namespace CGameEngine
{
	struct DrawPath;
	class TextureContainer;

    class FontManager
    {
        public:
			static FontManager& getInstance()
			{
				static FontManager instance;
				return instance;
			}

            ActiveFont* initFont(std::string identifier, const char* font, int size, char charStart = FIRST_PRINTABLE_CHAR, char charEnd = LAST_PRINTABLE_CHAR);
            void draw(ActiveFont* font, const char* s, glm::vec2 position, float scaling, float depth, glm::ivec4 tint, int just = Justification::Left);
            void targetedDraw(DrawPath* p, ActiveFont* font, const char* s, glm::vec2 position, float scaling, float depth, glm::ivec4 tint, int just = Justification::Left);

        private:
            FontManager();
            ~FontManager();
            void doDraw(DrawPath* p, ActiveFont* font, const char* s, glm::vec2 position, float scaling, float depth, glm::ivec4 tint, int just = Justification::Left);
            std::vector<int>* createRows(std::vector<glm::ivec4>* rects, int rectsLength, int r, int padding, int& w);

            TextureContainer* m_txContainer = nullptr;
            std::map<std::string, ActiveFont*> m_fonts; // <font name, font obj>
    };
}

#endif // FONTMANAGER_H
