#include "Slider.h"

namespace CGameEngine
{
    Slider::Slider(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL /*= PERMANENT_GUI_ELEMENT*/)
    {
        m_frame = frame;
        m_texture = glText;
        m_destRect = destRect;
        m_range = glm::ivec2(0, 100);
        m_color = color;
        //m_TTL = TTL;
        m_elementType = GUIElements::Slider;
    }

    bool Slider::hasCollision(const glm::vec2& posVec, int action /*= 0*/, bool isKeyHeld /*= false*/)
    {
        // catch to ensure disabled objects cannot be touched
        if(m_isDisabled || !m_isVisible) { return false; }

        if(isInside(posVec, m_centeredDestRect))
        {
            switch (action)
            {
                case GUIMouse::LeftClick:
                {
                    float incVal = convertV2ScrToWorld(posVec, e_screenWidth, e_screenHeight).x;
                    incVal -= m_xBounds[0]; // distance between click and base value
                    incVal /= (m_xBounds[1] - m_xBounds[0]); // percentage decimal of completion
                    incVal *= m_range[1]; // multiplied by max value to get total value
                    setValue( static_cast<int>(incVal) );
                    break;
                }
                case GUIMouse::MWheelDown:
                    setValue(m_value-1);
                    break;
                case GUIMouse::MWheelUp:
                    setValue(m_value+1);
                    break;
            }

            // something happened
            return true;
        }

        // else
        return false;
    }

    void Slider::setValue(int val)
    {
        if(val <= m_range[0]) { m_value = m_range[0]; } // keep above minimum
        else if(val >= m_range[1]) { m_value = m_range[1]; } // keep below maximum
        else { m_value = val; }
        m_needsUpdate = true;
		Logger::getInstance().Log(Logs::DEBUG, "Slider::setValue()", "Value is {}.", m_value);
    }

    bool Slider::update(float deltaTime /*= 0.0f*/)
    {
        // used for changes in position/size/etc
        if(m_needsUpdate)
        {
            m_needsUpdate = false;
            if(m_texture) { m_textureAddress = m_texture->getAddress(); }
            updateAlternateCoordinates(true);
            updateTextCentering(deltaTime);

			Logger::getInstance().Log(Logs::VERBOSE, Logs::GUI, "Slider::update()", "Slider CDR: [{}, {}, {}, {}]", m_centeredDestRect.x, m_centeredDestRect.y, m_centeredDestRect.z, m_centeredDestRect.w);

            // computing the X "bounds" for slider
            m_xBounds[0] = m_destRect.x - (m_centeredDestRect.z * m_usableBoundsPct[0]);
            m_xBounds[1] = m_destRect.x + (m_centeredDestRect.z * m_usableBoundsPct[1]);// + (m_centeredDestRect.z * 0.05f);
			Logger::getInstance().Log(Logs::VERBOSE, Logs::GUI, "Slider::update()", "x-Bounds ({} - {}), origin {}", m_xBounds[0], m_xBounds[1], m_destRect.x);

            // measuring tick distance
            m_stepTicks = (m_xBounds[1] - m_xBounds[0]) / m_range[1];
			Logger::getInstance().Log(Logs::VERBOSE, Logs::GUI, "Slider::update()", "m_stepTicks [{}]", m_stepTicks);


            /// Slider Tick Calculation
            // calculate width/height based on provided slider size percentage
            glm::vec4 tickDestRect = glm::vec4(0.0f);
            TextureAddress txtr = {0,0};
            if(m_isFillSlider) // filling slider
            {
                // setting initial "width" based on ticks
                tickDestRect.z = m_stepTicks * m_value;
                tickDestRect.x = m_xBounds[0];
                if(m_fillTexture) { txtr = m_fillTexture->getAddress(); }
            }
            else // regular slider
            {
                // prevent slider tick from being awkwardly thick
                tickDestRect.z = m_centeredDestRect.z * m_sliderSizePct.x;
                if(m_tickTexture && tickDestRect.z > m_tickTexture->getWidth()) { tickDestRect.z = m_tickTexture->getWidth(); }
                tickDestRect.x = m_xBounds[0] + (m_stepTicks * m_value) - (tickDestRect.z / 2.0f);
                if(m_tickTexture) { txtr = m_tickTexture->getAddress(); }
            }
            tickDestRect.w = m_centeredDestRect.w * m_sliderSizePct.y;
            tickDestRect.y = m_centeredDestRect.y - ((tickDestRect.w - m_centeredDestRect.w) / 2.0f);

            // create sliderTick, if not exists
            if(!m_sliderTick)
            {
                SubElement* tmp = nullptr;
                /// \TODO: Convert to emplace or clean up
                if(txtr.defined()) { tmp = new SubElement(tickDestRect, txtr); }
                else { tmp = new SubElement(tickDestRect); }
				if(tmp)
				{
					m_subElements.push_back(tmp);
					//m_subElements.emplace_back(tickDestRect, texID);
					m_sliderTick = m_subElements.back();
                }
            }
            else
            {
                m_sliderTick->destRect = tickDestRect;
                if(txtr.defined()) { m_sliderTick->textureAddress = txtr; }
            }

            Logger::getInstance().Log(Logs::VERBOSE, Logs::GUI, "Slider::update()", "Tick CDR: [{}, {}, {}, {}]", m_sliderTick->destRect.x, m_sliderTick->destRect.y, m_sliderTick->destRect.z, m_sliderTick->destRect.w);

            // get text centering
            updateTextCentering(deltaTime);
        }

        if(m_isPermanent) { return false; }
        else { return m_TTL->isExpired(); }
    }

    void Slider::defaultTextures()
    {
		m_texture = TextureManager::getInstance().getTexture("Assets/Slider.png");
		m_fillTexture = TextureManager::getInstance().getTexture("Assets/Slider_Fill_Red.png");
		if(m_isFillSlider && m_sliderTick) { m_sliderTick->texture = m_fillTexture;  m_sliderTick->textureAddress = m_fillTexture->getAddress(); }
        m_tickTexture = TextureManager::getInstance().getTexture("Assets/Slider_Tick.png");
        if(!m_isFillSlider && m_sliderTick) { m_sliderTick->texture = m_tickTexture; m_sliderTick->textureAddress = m_tickTexture->getAddress(); }
    }

    void Slider::destroySliderTick(bool rebuild /*= false*/)
    {
        for(unsigned int i = 0; i < m_subElements.size(); i++)
        {
            if(m_subElements[i] == m_sliderTick)
            {
                safeDelete(m_sliderTick);
                m_subElements.erase(m_subElements.begin() + i);
            }
        }

        //m_subElements.clear(); // empty list

        if(rebuild) { update(); }
    }
}
