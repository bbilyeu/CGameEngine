#ifndef FRAME_H
#define FRAME_H

#include "gui/gui_common.h"
#include "draw/TextureManager.h"
#include <unordered_map>

/// \TODO: Cleanup add___() calls (potentially switch statement for all but text)

namespace CGameEngine
{
	class Dropdown;
	class GUI;
	class GUIObject;
    class GUIRenderer;
    class PushButton;
    class RadioButton;
    class ScrollBox;
    class Slider;
    class FontManager;
    class TextObject;
    class TickBox;
    class Window;
    class Minimap;// : public GUIObject;
	class Wheel; // REALLY needed?

    class Frame
    {
        friend class GUIRenderer;

        public:
            Frame(GUI* parent, std::string internalName, const glm::vec4 destRect);
            ~Frame();
            const glm::vec4& getCenteredDestRect() const { return m_centeredDestRect; }
            const glm::vec4& getDestRect() const { return m_destRect; }
            const glm::vec3 getXYZ() const { return glm::vec3(m_destRect.x, m_destRect.y, 0.0f); }
            const glm::vec2 getDimensions() const { return glm::vec2(m_destRect.z, m_destRect.w); }

            const glm::vec4& getScreenDestRect() const { return m_screenDestRect; }
            const glm::vec3 getScreenXYZ() const { return glm::vec3(m_screenDestRect.x, m_screenDestRect.y, 0.0f); }
            const glm::vec2 getScreenDimensions() const { return glm::vec2(m_screenDestRect.z, m_screenDestRect.w); }

            GUIObject* getCollisionObject(const glm::vec2& mouseCoords, const glm::vec2& prevMouseCoords, int action, bool isKeyHeld);
            GUIObject* getObject(std::string name);
            Dropdown* addDropdown(std::string name, glm::vec4 destRect);
            PushButton* addPushButton(std::string name, glm::vec4 destRect);
            RadioButton* addRadioButton(std::string name, glm::vec4 destRect);
            ScrollBox* addScrollBox(std::string name, glm::vec4 destRect);
            Slider* addSlider(std::string name, glm::vec4 destRect);
            TextObject* addTextObject(std::string name, std::string text, glm::vec4 destRect);
            TickBox* addTickBox(std::string name, glm::vec4 destRect);
            GUIGlyph* getGlyph() { return m_glyph; }
            bool hasCollision(const glm::vec2& mouseCoords);
            const bool& isMoveable() const { return m_isMoveable; }
            std::unordered_map<std::string, GUIObject*>& getElements() { return m_frameElements; }
            const std::string& getInternalName() const { return m_internalName; }
            void centerInFrame(glm::vec4& destRect); // NOT scaling based on frame
            void resizeInFrame(glm::vec4& destRect); // frame scaling
            void setMoveable(bool val) { m_isMoveable = val; }
            void setPosition(float x, float y) { m_destRect.x = x; m_destRect.y = y; m_needsUpdate = true; }
            void setTexture(std::string path) { m_texture = TextureManager::getInstance().getTexture(path); m_needsUpdate = true; }
            void update(float deltaTime = 0.0f);

        private:
            glm::ivec4 m_color = COLOR_WHITE;
            glm::vec4 m_destRect = glm::vec4(0.0f);
            glm::vec4 m_centeredDestRect = glm::vec4(0.0f);
            glm::vec4 m_screenDestRect = glm::vec4(0.0f);
            glm::vec4 m_uvRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            Texture2D* m_texture = nullptr;
            TextureAddress m_textureAddress = {0,0};
            GUIGlyph* m_glyph = nullptr;
            GUI* m_parent = nullptr;

            bool m_isMoveable = false;
            bool m_needsUpdate = true;
            float m_angle = 0.0f;
            float m_borderThickness = 0.0f;
			std::string m_internalName = "";
            std::unordered_map<std::string, GUIObject*> m_frameElements;
    };
}

#endif // FRAME_H
