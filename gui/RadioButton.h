#ifndef RADIOBUTTON_H
#define RADIOBUTTON_H

#include "gui/GUIObject.h"

namespace CGameEngine
{
	class RadioButton : public GUIObject
    {
        public:
            RadioButton() {}
            RadioButton(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL = PERMANENT_GUI_ELEMENT);
            RadioButton(Frame* frame, std::string internalName, glm::vec4 destRect) : GUIObject(frame, internalName, destRect) { update(); }
            bool hasCollision(const glm::vec2& posVec, int action = 0, bool isKeyHeld = false) override;
            bool update(float deltaTime = 0.0f) override;
            void defaultTextures();
            RadioGroup* getRadioGroup() { return m_radioGroup; }
            const bool& isTicked() const { return m_isTicked; }
            void joinRadioGroup(RadioGroup* rg);
            void setTicked(bool ticked = false);
            void setTickedTexture(std::string path);
            void setUntickedTexture(std::string path);

        private:
            Texture2D* m_tickedTexture = nullptr;
            Texture2D* m_untickedTexture = nullptr;
            RadioGroup* m_radioGroup = nullptr;
            bool m_isTicked = false;
    };

	class RadioGroup
    {
        public:
            RadioGroup(std::string internalName) : m_radiobuttons(), m_internalName(internalName) {}
            RadioGroup(int val) { m_id = val; }
            ~RadioGroup();
            RadioButton* getTicked();
            bool addButton(RadioButton* rb);
            const int& getID() const { return m_id; }
            void tickButton(RadioButton* rb);
            const std::string& getInternalName() const { return m_internalName; }

        private:
            int m_id = 0;
            std::string m_internalName = "";
            std::vector<RadioButton*> m_radiobuttons;
    };
}

#endif // RADIOBUTTON_H
