#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <unordered_map>
#include <glm/glm.hpp>
#include "SDL2/SDL_keycode.h"
#include "SDL2/SDL_mouse.h"

namespace CGameEngine
{
    class InputManager
    {
        public:
            static InputManager& getInstance()
            {
				static InputManager instance;
				return instance;
            }
            ~InputManager();
            bool isKeyDown(unsigned int keyID); // returns true if key held down
            bool isKeyPressed(unsigned int keyID); // returns true if key is actually pressed
            const glm::vec2& getMouseCoords() const { return m_mouseCoords; }
            void pressKey(unsigned int keyID);
            void releaseKey(unsigned int keyID);
            void setMouseCoords(float x, float y);
            void update();

        private:
			InputManager() : m_keyMap(), m_previousKeyMap()
			{
//                m_keyMap = std::unordered_map<unsigned int, bool>();
//                m_previousKeyMap = std::unordered_map<unsigned int, bool>();
            }
            glm::vec2 m_mouseCoords = glm::vec2(0.0f);
            bool wasKeyDown(unsigned int keyID);
            std::unordered_map<unsigned int, bool> m_keyMap;
            std::unordered_map<unsigned int, bool> m_previousKeyMap;
    };
}
#endif // INPUTMANAGER_H
