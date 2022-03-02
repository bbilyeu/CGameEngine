#include "ToolTip.h"

namespace CGameEngine
{
   ToolTip::ToolTip(Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL /*= PERMANENT_GUI_ELEMENT*/)
    {
        m_texture = glText;
        m_destRect = destRect;
        m_color = color;
        //m_TTL = TTL;
        m_elementType = GUIElements::ToolTip;
        m_textJustification = Justification::Left;
        m_isVisible = false;
        m_maxWidth = e_screenWidth * 0.2f; // 20% of screen width
        m_textScaling = 0.85f;
        m_TTL = new Timer("", 1500);
    }

    void ToolTip::destroy()
    {
        // destroy from view, not actually destroying the object
        m_isVisible = false;
        m_isActive = false;
        m_destRect = glm::vec4(0.0f);
        m_centeredDestRect = m_destRect;
    }

    bool ToolTip::update(float deltaTime /*= 0.0f*/)
    {
         // if no hover object, we shouldn't be displaying
        if(!e_hover) { return true; }

        //Logger::getInstance().Log(Logs::DEBUG, "ToolTip::update()", "m_timeTilDisplay [{}] vs ticks [{}]", m_timeTilDisplay, ticks);

        // handle visibility logic
		if(m_TTL->isExpired() && !m_isVisible)
        {
            m_needsUpdate = true;
            m_isVisible = true;
        }

        if(m_needsUpdate)
        {
            // draw based on center, not a corner
            m_needsUpdate = false;

            // adjust size to that of original texture * specified size
            // "center" on top left corner
            m_centeredDestRect.z = m_destRect.z;
            m_centeredDestRect.w = m_destRect.w;
            m_centeredDestRect.x = m_destRect.x;
            m_centeredDestRect.y = m_destRect.y - m_centeredDestRect.w;

            /// bounds checking (to ensure we don't draw offscreen) ///

            // get quadrant max x,y
            int widthMax = e_screenWidth / 2;
            int heightMax = e_screenHeight / 2;

            // checking width
            if(m_centeredDestRect.x < 0 && abs(m_centeredDestRect.x) >= widthMax) { m_centeredDestRect.x = -(widthMax - 2.0f); }
            else if(m_centeredDestRect.x > 0 && (m_centeredDestRect.x + m_centeredDestRect.z) >= widthMax) { m_centeredDestRect.x = widthMax - m_centeredDestRect.z - 2.0f; }

            // checking height
            if(m_centeredDestRect.y < 0 && abs(m_centeredDestRect.y - m_centeredDestRect.w) >= heightMax) { m_centeredDestRect.y = -(heightMax - 2.0f); }
            else if(m_centeredDestRect.y > 0 && m_centeredDestRect.y >= heightMax) { m_centeredDestRect.y = heightMax - 2.0f; }

            /// \TODO: fix issue with it being too far to the left
            // update font "center" (actually top left)
            m_centeredTextXY.x = m_centeredDestRect.x + ( m_centeredDestRect.z * 0.1f - (e_guiFont->fontSize / 2.0f) );
            m_centeredTextXY.y = m_centeredDestRect.y + ( m_centeredDestRect.w * 0.9f - e_guiFont->fontHeight );
        }

        return false;
    }

    void ToolTip::defaultTextures()
    {
		m_texture = TextureManager::getInstance().getTexture("Assets/Dialog.png");
    }

    void ToolTip::setHoverObject(bool resetTimeTilDisplay /*= false*/)
    {
        // set to passed object
        //m_hover = e_hover;

        if(e_hover->getToolTipMessage() != "NULL")
        {
            m_isActive = true;
            m_needsUpdate = true;
            glm::vec4 destRect = glm::vec4(0.0f);
            glm::vec2 mouseVec = e_mouseVec;
            float scrWidth = e_screenWidth;
            float scrHeight = e_screenHeight;

            // make 0 the 'center' and draw slightly southeast of mouse
            destRect.x = (mouseVec.x - scrWidth / 2.0f) + (scrWidth * 0.0075f);
            destRect.y = ((scrHeight - mouseVec.y) - (scrHeight / 2.0f)) - (scrHeight * 0.01f); // invert y

            // get approximate size of tooltip based on length/size of text
            std::string str = e_hover->getToolTipMessage();
            float pixelSize = static_cast<float>(e_guiFont->fontSize);
            //float fontHeight = static_cast<float>(e_guiFont->fontHeight);

            // create necessary linebreaks (as needed)
            std::string tmp = str;
            int lastSpace = 0;
            int lastRow = 0;

            // insert newlines when necessary to avoid the tooltip
            //  from running past 20% of screen width
            for(unsigned int i = 0; i < str.length(); i++)
            {
                if(tmp.at(i) == ' ') { lastSpace = i; } // log last space
                if( ((i - lastRow) * pixelSize) > m_maxWidth ) { tmp.replace(lastSpace, 1, "\n"); lastRow = lastSpace; }
            }
            // hand newly formatted string back to 'str' variable
            str = tmp;

            // adjust width/height to that of final string
            glm::vec2 fontMeasurements = e_guiFont->measure(str, m_textScaling);
            destRect.z = fontMeasurements.x * 1.2f;
            destRect.w = fontMeasurements.y * 1.2f;

            // set final message and destRect
            m_text = str;
            m_destRect = destRect;
        }
        else
        {
            // hide tooltip and set inactive
            destroy();
        }

        if(resetTimeTilDisplay)
        {
            m_TTL->setExpired();
            m_isVisible = false;
        }
    }

}
