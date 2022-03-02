#include "ScrollBox.h"

namespace CGameEngine
{
    ScrollBox::ScrollBox(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL /*= PERMANENT_GUI_ELEMENT*/)
    {
        m_frame = frame;
        m_texture = glText;
        m_destRect = destRect;
        m_color = color;
        //m_TTL = TTL;
//        m_upTexture = TextureManager::getInstance().getTexture("Assets/ScrollBox_Upbutton.png");
//        m_downTexture = TextureManager::getInstance().getTexture("Assets/ScrollBox_Downbutton.png");
        m_elementType = GUIElements::ScrollBox;
        m_scrollTimer = new Timer("", HALF_SECOND);
    }

    bool ScrollBox::hasCollision(const glm::vec2& posVec, int action /*= 0*/, bool isKeyHeld /*= false*/)
    {
        // catch to ensure disabled objects cannot be touched
        if(m_isDisabled || !m_isVisible) { return false; }

        /// \TODO: Find way to allow use of buttons AND input together
        if(!m_noButtons && isInside(posVec, m_upButton->destRect))
        {
            switch (action)
            {
                case GUIMouse::LeftClick:
                    setValue(m_value + m_stepValue);
                    m_speed = 1.0f;
                    m_delay = 0;
                    break;
                case GUIMouse::MWheelUp:
                    setValue(m_value + m_stepValue);
                    break;
            }

            m_isInEditMode = false;
            // something happened
            return true;
        }
        else if(!m_noButtons && isInside(posVec, m_downButton->destRect))
        {
            switch (action)
            {
                case GUIMouse::LeftClick:
                    setValue(m_value - m_stepValue);
                    m_speed = 1.0f;
                    m_delay = 0;
                    break;
                case GUIMouse::MWheelDown:
                    setValue(m_value - m_stepValue);
                    break;
            }

            m_isInEditMode = false;
            return true;
        }
        /// \TODO: Check only scrollbox "textbox"
        else if(isInside(posVec, m_centeredDestRect))
        {
            // hovering over text box, not buttons
            switch (action)
            {
                case GUIMouse::LeftClick:
                    /// \TODO: accept input?
                    m_needsUpdate = true;
                    m_isInEditMode = true;
                    m_speed = 1.0f;
                    m_delay = 0;
                    break;
                case GUIMouse::MWheelDown:
                    setValue(m_value - m_stepValue);
                    break;
                case GUIMouse::MWheelUp:
                    setValue(m_value + m_stepValue);
                    break;
            }

            // something happened
            return true;
        }

        // else
        return false;
    }

    void ScrollBox::setNoButtons(bool val /*= true*/)
    {
        m_noButtons = val;
        m_needsUpdate = true;

        // cleanup (remove) buttons
        for(unsigned int i = 0; i < m_subElements.size();)
        {
            if(m_subElements.size() == 0) { i = 999; break; }
            else if(m_subElements[i] == m_upButton)
            {
                m_subElements[i] = m_subElements.back();
                m_subElements.pop_back();
                safeDelete(m_upButton);
            }
            else if(m_subElements[i] == m_downButton)
            {
                m_subElements[i] = m_subElements.back();
                m_subElements.pop_back();
                safeDelete(m_downButton);
            }
            else { i++; }
        }
    }

    void ScrollBox::setDownTexture(std::string path)
    {
        m_downTexture = TextureManager::getInstance().getTexture(path);
        if(m_downButton) { m_downButton->texture = m_downTexture; }
    }

    void ScrollBox::setUpTexture(std::string path)
    {
        m_upTexture = TextureManager::getInstance().getTexture(path);
        if(m_upButton) { m_upButton->texture = m_upTexture; }
    }

    void ScrollBox::setValue(float val, bool force /*= false*/)
    {
        // if valuePtr is set, use it exclusively
        if(m_valuePtr)
        {
            m_value = (*m_valuePtr);
            m_text = std::to_string(m_value);
            m_needsUpdate = true;
            return;
        }

        if(force)
        {
            if(val <= m_range[0]) { m_value = m_range[0]; }
            else if(val >= m_range[1]) { m_value = m_range[1]; }
            else { m_value = val; }
        }
        else
        {
            if(val <= m_range[0]) // minimum hit
            {
                if(m_rolloverEnabled && m_value == m_range[0]) { m_value = m_range[1]; }
                else if (m_rolloverEnabled && m_value != m_range[0]) { m_value = m_range[0]; }
                else { m_value = m_range[0]; }
            }
            else if(val >= m_range[1]) // maximum hit
            {
                if(m_rolloverEnabled && m_value == m_range[1]) { m_value = m_range[0]; }
                else if (m_rolloverEnabled && m_value != m_range[1]) { m_value = m_range[1]; }
                else { m_value = m_range[1]; }
            }
            else { m_value = val; }
        }
        m_text = std::to_string(m_value);
        m_needsUpdate = true;
    }

    bool ScrollBox::update(float deltaTime /*= 0.0f*/)
    {
        // if pointer passed to lock values, force an update
        if(m_valuePtr) { setValue(m_value); }

        // used for changes in position/size/etc
        if(m_needsUpdate)
        {
            m_needsUpdate = false;
            if(m_texture) { m_textureAddress = m_texture->getAddress(); }
            updateAlternateCoordinates(true);
            updateTextCentering(deltaTime);

            /// upButton definition
            glm::vec4 upDestRect = glm::vec4(0.0f);
            upDestRect.z = m_centeredDestRect.z * 0.6;
            upDestRect.w = m_centeredDestRect.w * 0.35;
            upDestRect.x = m_centeredDestRect.x + m_centeredDestRect.z;
            upDestRect.y = m_centeredDestRect.y + (m_centeredDestRect.w / 2.0f);
            if(!m_upButton && !m_noButtons)
            {
                if(m_upTexture) { m_upButton = new SubElement(upDestRect, m_upTexture->getAddress()); }
                else { m_upButton = new SubElement(upDestRect); }
                m_subElements.push_back(m_upButton);
            }
            else if(!m_noButtons) { m_upButton->destRect = upDestRect; }

            /// downButton definition
            glm::vec4 downDestRect = upDestRect;
            downDestRect.y = m_centeredDestRect.y + (m_centeredDestRect.w / 2.0f) - downDestRect.w;
            if(!m_downButton && !m_noButtons)
            {
                if(m_downTexture) { m_downButton = new SubElement(upDestRect, m_downTexture->getAddress()); }
                else { m_downButton = new SubElement(upDestRect); }
                m_subElements.push_back(m_downButton);
            }
            else if(!m_noButtons) { m_downButton->destRect = downDestRect; }

            // account for floating point
            char buffer[32];
            std::string formatString = "%3." + std::to_string(m_precision) + "f";
            if(m_stepValue - static_cast<int>(m_stepValue) != 0.0f) { sprintf(buffer, formatString.c_str(), m_value); } // decimal number
            else { sprintf(buffer, "%3.0f", m_value); } // whole number
            m_text = buffer;
            trim(m_text);

            // get text centering
            updateTextCentering(deltaTime);
        }

        // handle the "click and hold" functionality
        if(!m_noButtons && m_inputManager->isKeyDown(SDL_BUTTON_LEFT))
        {
            glm::vec2 mouseVec = e_mouseVec;
            if( isInside(mouseVec, m_upButton->destRect) ||
                isInside(mouseVec, m_downButton->destRect) )
            {
                if(m_scrollTimer->isExpired()) { m_scrollTimer->restart(); }
                else
                {
                    //std::cout<<"Scrollbox Update:\tm_delay ["<<m_delay<<"], ticks ["<<ticks<<"], m_speed ["<<m_speed<<"]\n";
                    m_speed *= 0.85f; // reducing delay in increments
                    if(m_speed < 0.1f) { m_speed = 0.1f; } // floor value
                    m_scrollTimer->addTime(HALF_SECOND * m_speed); // time until next increment
                    if(isInside(e_mouseVec, m_upButton->destRect))
                    {
                        // holding down the "up" button
                        setValue(m_value+1);
                    }
                    else if(isInside(mouseVec, m_downButton->destRect))
                    {
                        // holding down the "down" button
                        setValue(m_value-1);
                    }
                }
            }
        }

        if(m_isPermanent) { return false; }
        else { return m_TTL->isExpired(); }
    }

    void ScrollBox::defaultTextures()
    {
		m_texture = TextureManager::getInstance().getTexture("Assets/ScrollBox_Text.png");
		m_downTexture = TextureManager::getInstance().getTexture("Assets/ScrollBox_Downbutton.png");
		if(m_downButton) { m_downButton->texture = m_downTexture; m_downButton->textureAddress = m_downTexture->getAddress(); }
		m_upTexture = TextureManager::getInstance().getTexture("Assets/ScrollBox_Upbutton.png");
		if(m_upButton) { m_upButton->texture = m_upTexture; m_upButton->textureAddress = m_upTexture->getAddress(); }
    }
}
