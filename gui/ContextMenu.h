#ifndef CONTEXTMENU_H
#define CONTEXTMENU_H

#include "gui/GUIObject.h"

namespace CGameEngine
{
    class ContextMenu : public GUIObject
    {
        public:
            ContextMenu() {}
            ContextMenu(const glm::vec4 destRect, std::vector<std::string> elements, int* returnVal, float TTL = 1500.0f);
            //ContextMenu(std::string jsonData);
            bool update(float deltaTime = 0.0f) override;
            void defaultTextures();
            bool hasCollision(const glm::vec2& posVec, int action = 0, bool isKeyHeld = false) override;


        private:
            std::vector<std::string> m_elements; // list of elements
            Texture2D* m_elementTexture = nullptr;
            Texture2D* m_hoverTexture = nullptr;
            int m_hoverIndex = 0;
            int* m_returnValue = nullptr;
            uint32_t m_expire = 0;
    };
}

#endif // CONTEXTMENU_H
