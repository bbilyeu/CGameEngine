#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include "gui/GUIObject.h"

namespace CGameEngine
{
    class PushButton : public GUIObject
    {
        public:
            PushButton() {}
            PushButton(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL = PERMANENT_GUI_ELEMENT);
            PushButton(Frame* frame, std::string internalName, glm::vec4 destRect) : GUIObject(frame, internalName, destRect) { update(); }
            bool hasCollision(const glm::vec2& posVec, int action = 0, bool isKeyHeld = false) override;
            bool update(float deltaTime = 0.0f) override;
            //void bindFunction(void func()) { m_onClickFunc = func; }
            void defaultTextures();

        private:
			/// \NOTE: m_texture provided by GUIObject and is the 'active' texture
            Texture2D* m_baseTexture = nullptr;
            Texture2D* m_clickTexture = nullptr;
            Texture2D* m_hoverTexture = nullptr;
            //std::function<void()> m_onClickFunc; // declaring function pointer
            float m_timeTilBaseTexture = 0.0f; // MS
    };
}

#endif // PUSHBUTTON_H
