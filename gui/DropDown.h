#ifndef DROPDOWN_H
#define DROPDOWN_H

#include "gui/GUIObject.h"

namespace CGameEngine
{
	class Dropdown : public GUIObject
    {
        public:
            Dropdown() {}
            Dropdown(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL = PERMANENT_GUI_ELEMENT);
            Dropdown(Frame* frame, std::string internalName, glm::vec4 destRect);
            const glm::vec4& getPopoutDestRect() const { return m_popoutDestRect; }
            bool update(float deltaTime = 0.0f) override;
            void defaultTextures();
            TileSheet* getIconSpriteSheet() { return m_iconSpriteSheet; }
            const bool hasOnlyIcons() const { if(m_iconSpriteSheet && m_elements.size() < 2) { return true; } /*else*/ return false; }
            const bool& isExpanded() const { return m_isExpanded; }
            const int& getIconPosition() const { return m_iconPosition; }
            //const int getIndexByIconUV(glm::vec4& uv) const;
            const int getIndexByString(std::string str) const;
            const int& getMaxDisplayed() const { return MAX_SHOWN; }
            const uint& getSelectedElementIndex() const { return m_selectedElement; }
            const std::vector<std::string>& getElements() const { return m_elements; }
            const std::string getSelectedElement() { if(m_elements.size() == 0) { return "NULL"; } else {return m_elements[m_selectedElement]; } }
            void addElements(std::vector<std::string> elements); // add elements via vector
            void clearElements() { m_elements.clear(); m_selectedElement = 0; m_needsUpdate = true; }
            void setElements(std::vector<std::string> elements) { m_elements.clear(); addElements(elements); m_selectedElement = 0; m_needsUpdate = true; } // clears before adding
            void setSelectedElement(int index) { m_selectedElement = index; m_needsUpdate = true; }

        private:
            glm::vec4 m_popoutDestRect = glm::vec4(0.0f);
            bool m_isExpanded = false; // is popout expanded?
            int m_iconPosition = GUIPositions::Left;
            unsigned int m_selectedElement = 0; // current slot
            uint32_t m_expandedTTL = 0; // used to close expanded view after X seconds
            std::vector<std::string> m_elements; // list of elements
            const int MAX_SHOWN = 5; // max elements to show
    };

    class DropdownPopout : public GUIObject
    {
        public:
            DropdownPopout() {}
            DropdownPopout(Dropdown* ddParent, float TTL = 3000.0f);
            bool update(float deltaTime = 0.0f);
            void defaultTextures();
            bool hasCollision(const glm::vec2& posVec, int action = 0, bool isKeyHeld = false) override;
            const int& getNumberDisplayed() const { return m_numDisplayed; }
            const Dropdown* getParent() const { return m_ddParent; }
            //const std::vector<SubElement*>& getElementBoxes() const { return m_elementBoxes; }

        private:
            Dropdown* m_ddParent = nullptr;
            Texture2D* m_elementTexture = nullptr;
            Texture2D* m_hoverTexture = nullptr;
            Texture2D* m_hoverSelectedTexture = nullptr;
            Texture2D* m_selectedTexture = nullptr;
            TileSheet* m_iconSpriteSheet = nullptr;
            float m_minY = 0.0f;
            float m_maxY = 0.0f;
            float m_scrollingSpeed = 0.0f;
            int m_hoverIndex = 0;
            int m_iconPosition = GUIPositions::Left;
            int m_numDisplayed = 0;
            int m_scrollingDirection = 1; // -1 down, 1 up
            //std::vector<ListItem> m_elementBoxes;

            const float MAX_SCROLLSPEED = 20.0f;
    };
}

#endif // DROPDOWN_H
