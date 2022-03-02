#ifndef SCROLLBOX_H
#define SCROLLBOX_H

#include "gui/GUIObject.h"

namespace CGameEngine
{
    class ScrollBox : public GUIObject
    {
        public:
            ScrollBox() {}
            ScrollBox(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL = PERMANENT_GUI_ELEMENT);
            ScrollBox(Frame* frame, std::string internalName, glm::vec4 destRect) : GUIObject(frame, internalName, destRect) { update(); }
            ~ScrollBox() { safeDelete(m_scrollTimer); }
            bool hasCollision(const glm::vec2& posVec, int action = 0, bool isKeyHeld = false) override;
            bool update(float deltaTime = 0.0f) override;
            void defaultTextures();
            const float& getValue() const { return m_value; }
            float* getValuePtr() { return &m_value; }
            void decrease() { setValue(m_value-m_stepValue); }
            void increment() { setValue(m_value+m_stepValue); }
            void setDownTexture(std::string path); // down arrow
            void setMaxValue(float val) { m_range[1] = val; }
            void setMinValue(float val) { m_range[0] = val; }
            void setNoButtons(bool val = true);
            void setPrecision(uint8_t val) { m_precision = val; }
            void setRange(float minVal, float maxVal) { m_range[0] = minVal; m_range[1] = maxVal; m_needsUpdate = true; }
            void setRolloverEnabled(bool val = true) { m_rolloverEnabled = val; }
            void setStepValue(float val) { m_stepValue = val; }
            void setUpTexture(std::string path); // up arrow
            void setValue(float val, bool force = false);
            void setValuePtr(float* val = nullptr) { m_valuePtr = val; m_needsUpdate = true; }

        private:
            glm::vec2 m_range = glm::ivec2(0, 100); // i.e. 0 to 100
            Texture2D* m_upTexture = nullptr;
            Texture2D* m_downTexture = nullptr;
            SubElement* m_upButton = nullptr;
            SubElement* m_downButton = nullptr;
            Timer* m_scrollTimer = new Timer("", HALF_SECOND);
            void redrawElements(); // destroy and recreate textbox
            bool m_noButtons = false;
            bool m_rolloverEnabled = false;
            float m_speed = 1.0f; // increase/decrease speed while holding
            float m_stepValue = 1.0f; // amount to increase per click/scroll
            float m_value = 0.0f;
            float* m_valuePtr = nullptr;
            uint8_t m_precision = 1;
            uint32_t m_delay = 0; // used to delay adjustments while holding
    };
}

#endif // SCROLLBOX_H
