#include "ContextMenu.h"
#include "draw/Texture2D.h"

namespace CGameEngine
{
    ContextMenu::ContextMenu(const glm::vec4 destRect, std::vector<std::string> elements, int* returnVal, float TTL /*= THREE_SECONDS*/)
    {
        m_textureAddress = TextureManager::getInstance().getTexture("Assets/Dropdown_Element.png")->getAddress();
        m_destRect = destRect;
        m_destRect.w = e_guiFont->fontHeight * 1.2f;
        //m_destRect.y += (m_destRect.w / 2.0f);
        //m_TTL = TTL;
        m_elements = elements;
        m_elementType = GUIElements::Context;
        m_returnValue = returnVal;

        // 2.5 seconds
        if(!m_TTL) { m_TTL = new Timer("", 2500); }
        m_TTL->restart();

        // adjust size to that of original object
        m_centeredDestRect = m_destRect;
        //m_centeredDestRect.x = m_destRect.x - m_centeredDestRect.z / 2.0f;

        // expand to number of elements
        m_centeredDestRect.w *= m_elements.size();

        // place 'center' so top list item lines up with dropdown object
        m_centeredDestRect.y = m_destRect.y - m_centeredDestRect.w;

        // load textures
        //m_elementTexture = TextureManager::getInstance().getTexture("Assets/Dropdown_Element.png");
        //m_hoverTexture = m_elementTexture;

        // loop to tie creation logic together
        for(unsigned int i = 0; i < m_elements.size(); i++)
        {
            // texture setting
			TextureAddress txtr = m_textureAddress;
            if(i == m_hoverIndex) { txtr = m_hoverTexture->getAddress(); }

            // destRect assembly
            glm::vec4 dr = glm::vec4(m_centeredDestRect.x, (m_destRect.y - (m_destRect.w * (i+1))), m_destRect.z, m_destRect.w);

            // centering text
            glm::vec2 textCenter = glm::vec2(0.0f);
            glm::vec4 iconDestRect = glm::vec4(0.0f); // 0.0f
            textCenter.x = m_centeredDestRect.x + (dr.z * 0.1f);// + (dr.z / 2.0f); // add half of the width to bottom-left x
            textCenter.y = dr.y + ((dr.w - e_guiFont->fontHeight) / 2.0f); // add half of height minus font height to bottom-left y

            // create box
            SubElement* tmp = new SubElement(dr, txtr);
            if(tmp)
            {
				m_subElements.push_back(tmp);

				// add text, if exists
				if(i < m_elements.size()) { m_subElements.back()->setText(m_elements[i], textCenter); }
            }
        }
    }

    bool ContextMenu::update(float deltaTime /*= 0.0f*/)
    {
        // reached time limit
        if(m_TTL->isExpired()) { Logger::getInstance().Log(Logs::DEBUG, "ContextMenu::update()", "Returning true."); return true; }

        // used for changes in position/size/etc
        if(m_needsUpdate)
        {
            m_needsUpdate = false;

            /// creating boxes
            int iterations = m_subElements.size();

            // loop to tie creation logic together
            for(unsigned int i = 0; i < iterations; i++)
            {
                // destRect assembly
                glm::vec4 destRect = glm::vec4(m_centeredDestRect.x, (m_destRect.y - (m_destRect.w * (i+1))), m_destRect.z, m_destRect.w);

                // centering text
                glm::vec2 textCenter = glm::vec2(0.0f);
                glm::vec4 iconDestRect = glm::vec4(0.0f); // 0.0f
                textCenter.x = m_centeredDestRect.x + (destRect.z * 0.1f);// + (destRect.z / 2.0f); // add half of the width to bottom-left x
                textCenter.y = destRect.y + ((destRect.w - e_guiFont->fontHeight) / 2.0f); // add half of height minus font height to bottom-left y

                m_subElements[i]->destRect = destRect;
                m_subElements[i]->textCenter = textCenter;
                m_subElements[i]->textJust = Justification::Left;
                if(i == m_hoverIndex) { m_subElements[i]->textColor = COLOR_YELLOW; }
                else { m_subElements[i]->textColor = COLOR_WHITE; }
            }

            // get text centering
            //updateTextCentering(deltaTime);
            //m_centeredTextXY = m_ddParent->getCenteredTextXY();
            //m_centeredTextXY.y = m_centeredDestRect.y + ((m_centeredDestRect.w - e_guiFont->fontHeight) / 2.0f); // add half of height minus font height to bottom-left y
        }

        // else
        return false;
    }

    void ContextMenu::defaultTextures()
    {
		m_textureAddress = TextureManager::getInstance().getTexture("Assets/Dropdown_Element.png")->getAddress();
    }

    bool ContextMenu::hasCollision(const glm::vec2& posVec, int action /*= 0*/, bool isKeyHeld /*= false*/)
    {
        // catch to ensure disabled objects cannot be touched
        if(m_isDisabled || !m_isVisible) { return false; }

        if(isInside(posVec, m_centeredDestRect))
        {
            // handle mouse hover/click
            for(unsigned int i = 0; i < m_subElements.size(); i++)
            {
                // check coords per listitem
                if(isInside(posVec, m_subElements[i]->destRect))
                {
					 // reset time limit
                    m_TTL->restart();

                    // set new hover element
                    if(m_hoverIndex != i)
                    {
                        m_hoverIndex = i;
                        m_needsUpdate = true;
                    }

                    // set new selected element
                    if(action == GUIMouse::LeftClick)
                    {
                        (*m_returnValue) = i;
						Logger::getInstance().Log(Logs::VERBOSE, Logs::GUI, "ContextMenu::hasCollision()", "m_returnValue [{}], i [{}]", (*m_returnValue), i);
                        return true;
                    }
                }
            }
        }
        else // clicked outside
        {
            if(action == GUIMouse::LeftClick) { return true; }
        }

        // else
        return false;
    }
}
