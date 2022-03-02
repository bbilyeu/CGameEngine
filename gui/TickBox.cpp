#include "TickBox.h"

namespace CGameEngine
{
   TickBox::TickBox(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL /*= PERMANENT_GUI_ELEMENT*/)
    {
        m_frame = frame;
        m_texture = glText;
        m_destRect = destRect;
        m_color = color;
        //m_TTL = TTL;
        m_hoverTexture = TextureManager::getInstance().getTexture("Assets/TickBox_Hover.png");
        m_tickedTexture = TextureManager::getInstance().getTexture("Assets/TickBox_Ticked.png");
        m_untickedTexture = TextureManager::getInstance().getTexture("Assets/TickBox_Unticked.png");
    }

    bool TickBox::hasCollision(const glm::vec2& posVec, int action /*= 0*/, bool isKeyHeld /*= false*/)
    {
        // catch to ensure disabled objects cannot be touched
        if(m_isDisabled || !m_isVisible) { return false; }

        if(isInside(posVec, m_centeredDestRect))
        {
            if(action == GUIMouse::LeftClick)
            {
                if(!m_isTicked) { setTicked(true); } else { setTicked(false); } // invert tick/untick
                onClickEvent();
            }
            else { m_texture = m_hoverTexture; } // hovering

            return true;
        }

        // else
        return false;
    }

    bool TickBox::update(float deltaTime /*= 0.0f*/)
    {
        // used for changes in position/size/etc
        if(m_needsUpdate)
        {
            m_needsUpdate = false;
            if(m_texture) { m_textureAddress = m_texture->getAddress(); }
            updateAlternateCoordinates(true);
            updateTextCentering(deltaTime);
        }
        else
        {
            if(e_hover != this && m_texture->getAddress() == m_hoverTexture->getAddress() && m_isTicked == false)
            {
                // if not colliding, but still using another texture
                if(m_isTicked) { m_texture = m_tickedTexture; }
                else { m_texture = m_untickedTexture; }
                m_needsUpdate = true;
            }
        }

        if(m_isPermanent) { return false; }
        else { return m_TTL->isExpired(); }
    }

    void TickBox::defaultTextures()
    {
		m_hoverTexture = TextureManager::getInstance().getTexture("Assets/TickBox_Hover.png");
        m_tickedTexture = TextureManager::getInstance().getTexture("Assets/TickBox_Ticked.png");
        m_untickedTexture = TextureManager::getInstance().getTexture("Assets/TickBox_Unticked.png");
        m_texture = (m_isTicked) ? m_tickedTexture : m_untickedTexture;
    }

    void TickBox::setTicked(bool val /*= false*/)
    {
        m_isTicked = val;
        if(m_isTicked) { m_texture = m_tickedTexture; }
        else { m_texture = m_untickedTexture; }
    }

    void TickBox::setTickedTexture(std::string path)
    {
        m_tickedTexture = TextureManager::getInstance().getTexture(path);
    }

    void TickBox::setUntickedTexture(std::string path)
    {
        m_untickedTexture = TextureManager::getInstance().getTexture(path);
    }
}
