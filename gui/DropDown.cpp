#include "DropDown.h"

namespace CGameEngine
{
	Dropdown::Dropdown(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL /*= PERMANENT_GUI_ELEMENT*/)
    {
        m_frame = frame;
        m_texture = glText;
        m_destRect = destRect;
        m_color = color;
        //m_TTL = TTL;
        m_onClickEvent = [&] { if(!m_isExpanded) { safeDelete(e_dropdownPopout); e_dropdownPopout = new DropdownPopout(this); } };
        m_elementType = GUIElements::Dropdown;
    }

    Dropdown::Dropdown(Frame* frame, std::string internalName, glm::vec4 destRect)
		: GUIObject(frame, internalName, destRect)
	{
		update();
		m_onClickEvent = [&] { if(!m_isExpanded) { safeDelete(e_dropdownPopout); e_dropdownPopout = new DropdownPopout(this); } };
	}

    const int Dropdown::getIndexByString(std::string str) const
    {
        for(unsigned int i = 0; i < m_elements.size(); i++)
        {
            if(m_elements[i] == str) { return i; }
        }

        // else
        return 0;
    }

    bool Dropdown::update(float deltaTime /*= 0.0f*/)
    {
        // used for changes in position/size/etc
        if(m_needsUpdate)
        {
            // temporary size/loc variable
            glm::vec4 destRect = m_destRect;

            // draw based on center, not a corner
            m_needsUpdate = false;

//            if(m_frame)
//            {
//                // adjust size to relative portions of the frame
//                resizeInFrame(destRect);
//                m_centeredDestRect.z = destRect.z;
//                m_centeredDestRect.w = destRect.w;
//            }
//            else
//            {
//                // adjust size to that of original texture
//                m_centeredDestRect.z = m_texture->getWidth() * destRect.z;
//                m_centeredDestRect.w = m_texture->getHeight() * destRect.w;
//            }
//
//            m_centeredDestRect.x = destRect.x - m_centeredDestRect.z / 2.0f;
//            m_centeredDestRect.y = destRect.y - m_centeredDestRect.w / 2.0f;

            updateAlternateCoordinates(true);
            updateTextCentering(deltaTime);

            if(m_elements.size() > 0) { m_text = m_elements[m_selectedElement]; }
            else { m_selectedElement = 0; m_text = "NULL"; } // reset on empty
        }

        if(m_isPermanent) { return false; }
        else { return m_TTL->isExpired(); }
    }

    void Dropdown::defaultTextures()
    {
		m_texture = TextureManager::getInstance().getTexture("Assets/Dropdown.png");
    }

    void Dropdown::addElements(std::vector<std::string> elements)
    {
        for(unsigned int i = 0; i < elements.size(); i++)
        {
            m_elements.push_back(elements[i]);
        }

        m_needsUpdate = true;
    }

    DropdownPopout::DropdownPopout(Dropdown* ddParent, float TTL /*= 3000.0f*/)
    {
        // set parent elements
        m_ddParent = ddParent;
        m_textColor = m_ddParent->getTextColor();
        m_textScaling = m_ddParent->getTextScaling();
        m_subTextScaling = m_ddParent->getSubTextScaling();

        // spritesheet setting
        if(m_ddParent->getIconSpriteSheet() != nullptr)
        {
            m_iconSpriteSheet = m_ddParent->getIconSpriteSheet();
            m_iconPosition = m_ddParent->getIconPosition();
        }

        // destRect is effectively the "true" size of the parent dropdown
        m_destRect = m_ddParent->getDestRect();
        if(m_ddParent->getFrame()) { m_ddParent->getFrame()->centerInFrame(m_destRect); }
        m_destRect.z = m_ddParent->getCenteredDestRect().z;
        m_destRect.w = m_ddParent->getCenteredDestRect().w;
        m_destRect.y += (m_destRect.w / 2.0f); // to account for centerInFrame

        // determine elements displayed
        unsigned int numElements = m_ddParent->getElements().size();
        int maxDisplayed = m_ddParent->getMaxDisplayed();

        // if no text, check if only icons
        if(m_ddParent->hasOnlyIcons()) { numElements = m_iconSpriteSheet->getMaxIndex(); }

        // adjust size to that of original object
        m_centeredDestRect = m_destRect;
        m_centeredDestRect.x = m_destRect.x - m_centeredDestRect.z / 2.0f;

        // expand to number of elements or max shown, whichever is lower
        if(numElements >= maxDisplayed) { m_centeredDestRect.w *= maxDisplayed; m_numDisplayed = maxDisplayed; }
        else { m_centeredDestRect.w *= numElements; m_numDisplayed = numElements; }

        // place 'center' so top list item lines up with dropdown object
        m_centeredDestRect.y = m_destRect.y - m_centeredDestRect.w;

        // load textures
        m_texture = TextureManager::getInstance().getTexture("Assets/Dialog.png");
        m_elementTexture = TextureManager::getInstance().getTexture("Assets/Dropdown_Element.png");
        m_hoverTexture = TextureManager::getInstance().getTexture("Assets/Dropdown_Hover.png");
        m_hoverSelectedTexture = TextureManager::getInstance().getTexture("Assets/Dropdown_HoverSelected.png");
        m_selectedTexture = m_ddParent->getTexture();

        // limit scrolling
        m_minY = m_destRect.y; // downward
        m_maxY = m_destRect.y + (m_destRect.w * numElements) - (m_destRect.w * m_numDisplayed); // upward

        // misc sets then update
        m_color = m_ddParent->getColor();
        m_centeredTextXY = m_ddParent->getCenteredTextXY();
        m_destRect.y += m_destRect.w * m_ddParent->getSelectedElementIndex();
        m_elementType = GUIElements::DropdownPopout;
    }

    bool DropdownPopout::hasCollision(const glm::vec2& posVec, int action /*= 0*/, bool isKeyHeld /*= false*/)
    {
        // catch to ensure disabled objects cannot be touched
        if(m_isDisabled || !m_isVisible) { return false; }

        if(isInside(posVec, m_centeredDestRect))
        {
            // handle mouse wheel action
            if(action == GUIMouse::MWheelUp || action == GUIMouse::MWheelDown)
            {
                // set direction (up or down)
                if(action == GUIMouse::MWheelUp) { m_scrollingDirection = 1; } else { m_scrollingDirection = -1; }

                // increment speed
                m_scrollingSpeed = (m_scrollingSpeed + 1.0f) * 1.10f;
                if(m_scrollingSpeed > MAX_SCROLLSPEED) { m_scrollingSpeed = MAX_SCROLLSPEED; }

                m_needsUpdate = true;
            }

            // handle mouse hover/click
            for(unsigned int i = 0; i < m_subElements.size(); i++)
            {
                // check coords per listitem
                if(isInside(posVec, m_subElements[i]->destRect))
                {
                    // set new hover element
                    if(m_hoverIndex != i)
                    {
                        m_hoverIndex = i;
                        m_needsUpdate = true;
                    }
                    // set new selected element
                    if(action == GUIMouse::LeftClick)
                    {
                        m_ddParent->setSelectedElement(i);
                        m_ddParent->setVisible();
                        if(m_iconSpriteSheet)
                        {
                            m_ddParent->addIcon(m_subElements[i]->iconDestRect, m_subElements[i]->iconUVRect, m_subElements[i]->iconTextureAddress, m_iconPosition);
                        }
                        return true;
                    }
                }
            }
        }
        else // clicked outside
        {
            if(action == GUIMouse::LeftClick)
            {
                m_ddParent->setVisible();
                return true;
            }
        }

        // else
        return false;
    }

    bool DropdownPopout::update(float deltaTime /*= 0.0f*/)
    {
        /// NOTES:
        // m_destRect defines the list size based on the original element x/y/width/height
        // m_centeredDestRect defines the scissorBox and size of the dropdown

        // used for changes in position/size/etc
        if(m_needsUpdate || m_scrollingSpeed > 0.0f)
        {
            //std::cout<<"update: m_needsUpdate ["<<m_needsUpdate<<"], m_scrollingSpeed ["<<m_scrollingSpeed<<"]\n";
            m_needsUpdate = false;

            // adjust list placement
            if(m_scrollingSpeed > 0.0f)
            {
                m_destRect.y += m_scrollingSpeed * deltaTime * m_scrollingDirection;

                // decelerate scrolling
                float deceleration = m_scrollingSpeed * 0.08f * deltaTime;
                m_scrollingSpeed -= std::max(deceleration, 0.000075f);
            }
            else { m_scrollingSpeed = 0.0f; }

            // cap scrolling
            //if(m_subElements.size() < m_ddParent->getMaxDisplayed()) { m_destRect.y = m_minY; }
            if(m_destRect.y < m_minY) { m_destRect.y = m_minY; }
            else if(m_destRect.y > m_maxY) { m_destRect.y = m_maxY; }

            /// creating boxes
            bool isFirstPass = false;
            int iterations = m_subElements.size();
            std::vector<std::string> elementsText = m_ddParent->getElements();

            // if no elements or 1 element that is a label, BUILD THE THINGS
            if(m_subElements.size() == 0 || m_subElements.size() == 1 && m_subElements[0]->isLabel == true)
            {
                isFirstPass = true;
                iterations = elementsText.size();
            }

            // use icons as iterations setter
            if(iterations == 0 && m_iconSpriteSheet) { iterations = m_iconSpriteSheet->getMaxIndex(); }

            // loop to tie creation logic together
            for(unsigned int i = 0; i < iterations; i++)
            {
                // texture setting
                TextureAddress txtr = m_elementTexture->getAddress();
                if(i == m_ddParent->getSelectedElementIndex())
                {
                    if(i == m_hoverIndex) { txtr = m_hoverSelectedTexture->getAddress(); }
                    else { txtr = m_selectedTexture->getAddress(); }
                }
                else if(i == m_hoverIndex) { txtr = m_hoverTexture->getAddress(); }

                // destRect assembly
                glm::vec4 destRect = glm::vec4(m_centeredDestRect.x, (m_destRect.y - (m_destRect.w * (i+1))), m_destRect.z, m_destRect.w);

                // centering text
                glm::vec2 textCenter = glm::vec2(0.0f);
                glm::vec4 iconDestRect = glm::vec4(0.0f); // 0.0f
                textCenter.x = m_centeredDestRect.x + (destRect.z / 2.0f); // add half of the width to bottom-left x
                textCenter.y = destRect.y + ((destRect.w - e_guiFont->fontHeight) / 2.0f); // add half of height minus font height to bottom-left y

                if(m_iconSpriteSheet)
                {
                    // create uniform box
                    iconDestRect.z = std::min(nextBox(static_cast<int>(m_centeredDestRect.z / 5.0f)), nextBox(static_cast<int>(m_centeredDestRect.w / 5.0f)));
                    iconDestRect.w = iconDestRect.z;
                    iconDestRect.y = destRect.y + (destRect.w / 2.0f) - (iconDestRect.w / 2.0f);
                    iconDestRect.x = destRect.x + (destRect.z / 2.0f) - (iconDestRect.z / 2.0f);

                    // if box has text
                    if(i < elementsText.size())
                    {
                        // icon positioning and text offsetting
                        if(m_iconPosition == GUIPositions::Right)
                        {
                            iconDestRect.x = textCenter.x + e_guiFont->measure(elementsText[i], m_textScaling).x;
                            textCenter.x -= iconDestRect.z / 2.0f * 1.025f;
                        }
                        else // effectively all other positions become GUIPositions::Left
                        {
                            iconDestRect.x = textCenter.x - iconDestRect.z;
                            textCenter.x += iconDestRect.z * 0.5f;
                        }
                    }
                }

                // creating final box
                if(isFirstPass)
                {
                    // create box
                    /// \TODO: Convert to emplace or clean up
                    m_subElements.emplace_back(new SubElement(destRect, txtr));
                    //m_subElements.emplace_back(destRect, txtr);

                    // add text, if exists
                    if(i < elementsText.size()) { m_subElements.back()->setText(elementsText[i], textCenter); }

                    // add icon, if exists
                    if(m_iconSpriteSheet && i < m_iconSpriteSheet->getMaxIndex())
                    {
                        m_subElements.back()->setIcon(iconDestRect, m_iconSpriteSheet->getUVs(i), m_iconSpriteSheet->getTextureAddress(), m_iconPosition);
                    }
                }
                // merely adjusting it
                else
                {
                    m_subElements[i]->destRect = destRect;
                    m_subElements[i]->textCenter = textCenter;
                    m_subElements[i]->textureAddress = txtr;

                    // adjust icon, if exists
                    if(m_iconSpriteSheet)
                    {
                        m_subElements[i]->setIcon(iconDestRect, m_iconSpriteSheet->getUVs(i), m_iconSpriteSheet->getTextureAddress(), m_iconPosition);
                    }
                }
            }

            // get text centering
            m_centeredTextXY = m_ddParent->getCenteredTextXY();
            m_centeredTextXY.y = m_centeredDestRect.y + ((m_centeredDestRect.w - e_guiFont->fontHeight) / 2.0f); // add half of height minus font height to bottom-left y
        }

        if(m_isPermanent) { return false; }
        else { return m_TTL->isExpired(); }
    }

    void DropdownPopout::defaultTextures()
    {
		m_texture = TextureManager::getInstance().getTexture("Assets/Dialog.png");
        m_elementTexture = TextureManager::getInstance().getTexture("Assets/Dropdown_Element.png");
        m_hoverTexture = TextureManager::getInstance().getTexture("Assets/Dropdown_Hover.png");
        m_hoverSelectedTexture = TextureManager::getInstance().getTexture("Assets/Dropdown_HoverSelected.png");
        m_selectedTexture = m_ddParent->getTexture();
    }
}
