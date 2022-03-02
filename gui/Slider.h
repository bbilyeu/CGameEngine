#ifndef SLIDER_H
#define SLIDER_H

#include "gui/GUIObject.h"

namespace CGameEngine
{
    class Slider : public GUIObject
    {
        public:
            Slider() {}
            Slider(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL = PERMANENT_GUI_ELEMENT);
            Slider(Frame* frame, std::string internalName, glm::vec4 destRect) : GUIObject(frame, internalName, destRect) { update(); }
            ~Slider() { destroySliderTick(); }
            bool hasCollision(const glm::vec2& posVec, int action = 0, bool isKeyHeld = false) override;
            bool update(float deltaTime = 0.0f) override;
            void defaultTextures();
            const bool& isFillSlider() const { return m_isFillSlider; }
            void setFillSlider(bool val) { m_isFillSlider = val; destroySliderTick(true); }
            void setMaxValue(int val) { m_range[1] = val; destroySliderTick(true); }
            void setMinValue(int val) { m_range[0] = val; destroySliderTick(true); }
            void setRange(int minVal, int maxVal) { m_range[0] = minVal; m_range[1] = maxVal; m_needsUpdate = true; }
            void setSliderTickPct(const glm::vec2 val) { m_sliderSizePct = val; destroySliderTick(true); }
            void setValue(int val) override;
            void setUsableBoundsPct(glm::vec2 val) { m_usableBoundsPct = val; m_xBounds = glm::vec2(0.0f); m_needsUpdate = true; }
            void setXBounds(glm::vec2 bounds) { m_xBounds = bounds; m_needsUpdate = true; }

        private:
            glm::ivec2 m_range = glm::ivec2(0, 100); // i.e. 0 to 100
            glm::vec2 m_sliderSizePct = glm::vec2(0.1f, 1.0f);
            glm::vec2 m_usableBoundsPct = glm::vec2(0.33f);
            glm::vec2 m_xBounds = glm::vec2(0.0f);
            glm::vec4 m_sliderDestRect = glm::vec4(0.0f); /// must be a centered destRect!!!
            Texture2D* m_fillTexture = nullptr; // the slider fill texture
            Texture2D* m_tickTexture = nullptr; // the slider grab-able texture
            SubElement* m_sliderTick = nullptr;
            void destroySliderTick(bool rebuild = false); // pass with true to rebuild
            bool m_isFillSlider = true;
            float m_stepTicks = 0.0f;
    };
}

#endif // SLIDER_H
