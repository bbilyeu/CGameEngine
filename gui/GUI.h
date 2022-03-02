#ifndef GUI_H
#define GUI_H

#include "gui/gui_common.h"
#include "draw/Camera2D.h"
#include "gui/ContextMenu.h"
#include "gui/DropDown.h"
#include "gui/PushButton.h"
#include "gui/RadioButton.h"
#include "gui/ScrollBox.h"
#include "gui/Slider.h"
#include "gui/TextObject.h"
#include "gui/TickBox.h"
#include "gui/ToolTip.h"
#include "gui/Frame.h"
#include <SDL2/SDL_events.h>

/// \TODO: How does cursor blink actually make blinky?

namespace CGameEngine
{
    class GUI
    {
        friend class GUIRenderer;

        public:
            GUI();
            ~GUI();
            void dispose();
            void init(Window* window, bool drawDebug = false, std::string font = "", char fontSize = 13);
            bool buildUI(std::string filePath);
            void update(float deltaTime = 0.0f);
            //ContextMenu(GUI* parent, Texture2D* glText, const glm::vec4 destRect, std::vector<std::string> elements, int* returnVal, float TTL = 2500);
            ContextMenu* addContextMenu(glm::vec2 dimensions, std::vector<std::string> elements, int* retVal);
            ContextMenu* addContextMenu(glm::vec2 dimensions, std::vector<std::string> elements) { return addContextMenu(dimensions, elements, m_contextRetVal); }
            //DropdownPopout* getDropdownPopout() { return m_dropdownPopout; }
            Frame* addFrame(std::string name = "defaultFrame", glm::vec4 destRect = glm::vec4(0.0f));
            GUIObject* getObject(std::string name);
            RadioGroup* addRadioGroup();
            RadioGroup* getRadioGroup(int i) { if(m_radiogroups[i]) { return m_radiogroups[i]; } else { return nullptr; } }
            bool onSDLEvent(SDL_Event& evnt);
            Camera2D* getCamera2D() { return &m_guiCamera; }
            const GUIObject* getLastTouched() const { return m_lastTouchedObject; }
            const glm::vec2 getTextMeasurements(std::string str, float scaling);
            const glm::vec2& getMouseScroll() const { return m_scrollVec; }
            InputManager* getInputManager() { return m_inputManager; }
            //const char& getFontSize() const { return FONT_SIZE; }
            //const char& getFontHeight() const { return FONT_HEIGHT; } // store on renderer init
            int getContextReturnValue() { int r = (*m_contextRetVal); (*m_contextRetVal) = -1; return r; }
            const int& getScreenHeight() const { return e_screenHeight; }
            const int& getScreenWidth() const { return e_screenWidth; }
            std::unordered_map<std::string, Frame*>& getFrames() { return m_frames; }
            void calcTextSize(glm::vec4& passedVec, std::string text, float scaling = 1.0f);
            void draw();
            void loadTextures(); // load all textures
            void setDebug(bool val = true);
            void setToolTipEnabled(bool flag = true) { m_isToolTipEnabled = flag; }
            void setFallbackDefaultTextures(bool val = true) { m_fallbackDefault = val; }

        private:
            // resources
            Camera2D m_guiCamera;
            ConfigReader* m_config = nullptr;
            ContextMenu* m_contextMenu = nullptr;
            //DropdownPopout* m_dropdownPopout = nullptr;
            Frame* m_lastFrame = nullptr;
            Texture2D* m_tooltipTexture;
            GUIObject* m_lastTouchedObject = nullptr;
            InputManager* m_inputManager = nullptr;
            ToolTip* m_toolTip = nullptr;
            Window* m_window = nullptr;
            Timer* m_cursorBlinkTimer = nullptr;
            int* m_contextRetVal = nullptr;
            glm::vec2 m_prevMouseVec = glm::vec2(0.0f);
            glm::vec2 m_scrollVec = glm::vec2(0.0f);
            //std::pair<GUIObject*, uint32_t> m_hoverElement = std::make_pair( nullptr, 0 );
            std::unordered_map<std::string, CGameEngine::Frame*> m_frames; // <name, frame*>
            std::vector<RadioGroup*> m_radiogroups;

            // functions
            GUIObject* guiCollision(int action, bool& stillHasFocus);
            int getPixelSize();
            void toolTipLogic();
            void touchObject(GUIObject* obj, bool done = false);
            //std::string formatString(const float& pixelSize, std::string str);

            // variables
            bool m_drawDebug = false;
            bool m_fallbackDefault = true;
            bool m_isToolTipEnabled = true;
            bool m_isRunning = true;
            float m_maxToolTipWidth = 0.0f;
            std::string m_inputString = "";

            // configurable options
            int CURSOR_BLINK = 750; // milliseconds
            int HOVER_DELAY = 1000; // milliseconds
    };
}
#endif // GUI_H
