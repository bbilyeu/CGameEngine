#ifndef TICKBOX_H
#define TICKBOX_H

#include "gui/GUIObject.h"

namespace CGameEngine
{
    class TickBox : public GUIObject
    {
        public:
            TickBox() {}
            TickBox(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL = PERMANENT_GUI_ELEMENT);
            TickBox(Frame* frame, std::string internalName, glm::vec4 destRect) : GUIObject(frame, internalName, destRect) { update(); }
            bool hasCollision(const glm::vec2& posVec, int action = 0, bool isKeyHeld = false) override;
            bool update(float deltaTime = 0.0f) override;
            void defaultTextures();
            const bool& isTicked() const { return m_isTicked; }
            void setTicked(bool val = false);
            void setTickedTexture(std::string path);
            void setUntickedTexture(std::string path);

        private:
            Texture2D* m_hoverTexture = nullptr;
            Texture2D* m_tickedTexture = nullptr;
            Texture2D* m_untickedTexture = nullptr;
            bool m_isTicked = false;
    };
}

#endif // TICKBOX_H
