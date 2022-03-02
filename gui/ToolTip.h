#ifndef TOOLTIP_H
#define TOOLTIP_H

#include "gui/GUIObject.h"

namespace CGameEngine
{
    class ToolTip : public GUIObject
    {
        public:
            ToolTip() {}
            ToolTip(Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL = PERMANENT_GUI_ELEMENT);
            ToolTip(std::string jsonData);
            bool update(float deltaTime = 0.0f) override;
            const bool& isActive() const { return m_isActive; }
            //const int getTextLength() const { return m_text.length(); } // needed?
            //const glm::vec2 getTopLeftXY() const { return glm::vec2(m_centeredDestRect.x, m_destRect.y); } // needed?
            void defaultTextures();
            void destroy();
            void setHoverObject(bool resetTimeTilDisplay = false);
            void setMaxWidth(float w) { m_maxWidth = w; }
            void setHoverDelay(int val) { m_hoverDelay = val; }

        private:
            //GUIObject* m_hover = nullptr;
            bool m_isActive = false;
            int m_hoverDelay = 0;
            float m_maxWidth = 0.0f;
            uint32_t m_timeTilDisplay = 1500; // milliseconds
    };
}

#endif // TOOLTIP_H
