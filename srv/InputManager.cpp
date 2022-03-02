#include "srv/InputManager.h"

namespace CGameEngine
{
    InputManager::~InputManager()
    {
		m_previousKeyMap.clear();
		m_keyMap.clear();
    }

    bool InputManager::isKeyDown(unsigned int keyID)
    {
        if(m_keyMap.empty()) { return false; }
        else
        {
            // not using the associative array approach here because it
            // should not create a key if it wasn't pressed at some point
            auto it = m_keyMap.find(keyID);
            if(it == m_keyMap.end()) { return false; } // not found, so it can't be pressed
            else { return it->second; } // found the key, return the state
        }
    }

    bool InputManager::isKeyPressed(unsigned int keyID)
    {
        bool isPressed;
        if(isKeyDown(keyID) == true && wasKeyDown(keyID) == false)
        {
            return true;
        }

        return false;
    }

    void InputManager::pressKey(unsigned int keyID)
    {
        m_keyMap[keyID] = true;
    }

    void InputManager::releaseKey(unsigned int keyID)
    {
        m_keyMap[keyID] = false;
    }

    void InputManager::setMouseCoords(float x, float y)
    {
        m_mouseCoords.x = x;
        m_mouseCoords.y = y;
    }

    void InputManager::update()
    {
        m_previousKeyMap.clear();
        m_previousKeyMap = m_keyMap;
    }

    /// Private functions below ///////////////////////////////////////////////

    bool InputManager::wasKeyDown(unsigned int keyID)
    {
        auto it = m_previousKeyMap.find(keyID);
        if(it != m_previousKeyMap.end()) { return it->second; }

        return false;
    }
}
