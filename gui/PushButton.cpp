#include "PushButton.h"

namespace CGameEngine
{
    PushButton::PushButton(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL /*= PERMANENT_GUI_ELEMENT*/)
    {
        m_frame = frame;
        m_texture = glText;
        m_destRect = destRect;
        m_color = color;
        //m_TTL = TTL;
        m_baseTexture = m_texture;
        m_clickTexture = TextureManager::getInstance().getTexture("Assets/PushButton_Click.png");
        m_hoverTexture = TextureManager::getInstance().getTexture("Assets/PushButton_Hover.png");
        m_texture = m_baseTexture;
        m_elementType = GUIElements::PushButton;
        Logger::getInstance().Log(Logs::DEBUG, Logs::GUI, "PushButton::PushButton()", "m_baseTexture [{}, {}, {}], m_clickTexture [{}, {}, {}], m_hoverTexture [{}, {}, {}]",
			(bool)(m_baseTexture == nullptr), m_baseTexture->getAddress().containerHandle, m_baseTexture->getAddress().sliceNumber,
			(bool)(m_clickTexture == nullptr), m_clickTexture->getAddress().containerHandle, m_clickTexture->getAddress().sliceNumber,
			(bool)(m_hoverTexture == nullptr), m_hoverTexture->getAddress().containerHandle, m_hoverTexture->getAddress().sliceNumber);
    }

    bool PushButton::hasCollision(const glm::vec2& posVec, int action /*= 0*/, bool isKeyHeld /*= false*/)
    {
        // catch to ensure disabled objects cannot be touched
        if(m_isDisabled || !m_isVisible) { return false; }

        if(isInside(posVec, m_centeredDestRect))
        {
            if(action == GUIMouse::LeftClick)
            {
                m_texture = m_clickTexture;
                m_timeTilBaseTexture = 5.0f; // milliseconds
                onClickEvent();
            }
            else { m_texture = m_hoverTexture; } // hovering

            m_needsUpdate = true;
            return true;
        }

        /// else
        return false;
    }

    bool PushButton::update(float deltaTime /*= 0.0f*/)
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
            if(e_hover != this)
            {
				if(!m_baseTexture || !m_clickTexture || !m_hoverTexture)
				{
					Logger::getInstance().Log(Logs::WARN, Logs::GUI, "PushButton::update()",
						"baseTexture, clickTexture, or hoverTextre is null during update loop. Loading default textures.");
					defaultTextures();
				}
				else if(!m_texture || m_texture->getAddress() != m_baseTexture->getAddress())
				{
					// if not colliding, but still using another texture
					m_texture = m_baseTexture;
					m_textureAddress = m_baseTexture->getAddress();
					m_needsUpdate = true;
				}
            }
        }

        // resetting texture
        if(m_timeTilBaseTexture > 0.0f)
        {
            m_timeTilBaseTexture -= deltaTime;
            if(m_timeTilBaseTexture <= 0.0f)
            {
                m_timeTilBaseTexture = 0.0f;
                m_texture = m_baseTexture;
            }
        }

        if(m_isPermanent) { return false; }
        else { return m_TTL->isExpired(); }
    }

	void PushButton::defaultTextures()
	{
		m_baseTexture = TextureManager::getInstance().getTexture("Assets/PushButton.png");
		m_clickTexture = TextureManager::getInstance().getTexture("Assets/PushButton_Click.png");
        m_hoverTexture = TextureManager::getInstance().getTexture("Assets/PushButton_Hover.png");
        Logger::getInstance().Log(Logs::DEBUG, Logs::GUI, "PushButton::defaultTextures()", "m_baseTexture [{}, {}, {}], m_clickTexture [{}, {}, {}], m_hoverTexture [{}, {}, {}]",
			(bool)(m_baseTexture == nullptr), m_baseTexture->getAddress().containerHandle, m_baseTexture->getAddress().sliceNumber,
			(bool)(m_clickTexture == nullptr), m_clickTexture->getAddress().containerHandle, m_clickTexture->getAddress().sliceNumber,
			(bool)(m_hoverTexture == nullptr), m_hoverTexture->getAddress().containerHandle, m_hoverTexture->getAddress().sliceNumber);
		if(!m_baseTexture || !m_clickTexture || !m_hoverTexture) { Logger::getInstance().Log(Logs::FATAL, Logs::GUI, "PushButton::defaultTextures()", "Failed to load default textures! base [{}], click [{}], hover [{}]", (bool)(m_baseTexture == nullptr), (bool)(m_clickTexture == nullptr), (bool)(m_hoverTexture == nullptr)); }
		m_needsUpdate = true;
	}
}
