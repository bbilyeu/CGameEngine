#include "TextObject.h"

namespace CGameEngine
{
    TextObject::TextObject(Frame* frame, std::string text, float textScaling, const glm::vec4 destRect, const glm::ivec4 color, float TTL /*= PERMANENT_GUI_ELEMENT*/)
    {
        m_frame = frame;
        m_text = text;
        m_textScaling = textScaling;
        m_destRect = destRect;
        m_color = color;
        //m_TTL = TTL;
        m_elementType = GUIElements::TextObject;
    }

    bool TextObject::update(float deltaTime /*= 0.0f*/)
    {
        // used for changes in position/size/etc
        if(m_needsUpdate)
        {
            m_needsUpdate = false;
            if(m_texture) { m_textureAddress = m_texture->getAddress(); }
            updateAlternateCoordinates(true);

            // get text centering
            m_centeredTextXY.y = m_centeredDestRect.y + ((m_centeredDestRect.w - e_guiFont->fontHeight) / 2.0f); // add half of height minus font height to bottom-left y
            m_centeredTextXY.x = m_centeredDestRect.x + (m_centeredDestRect.z / 2.0f); // add half of the width to bottom-left x
            updateTextCentering(deltaTime, true);
        }

        if(m_isPermanent) { return false; }
        else { return m_TTL->isExpired(); }
    }
}
