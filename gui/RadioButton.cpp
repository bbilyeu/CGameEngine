#include "RadioButton.h"

namespace CGameEngine
{
    RadioButton::RadioButton(Frame* frame, Texture2D* glText, const glm::vec4 destRect, const glm::ivec4 color, float TTL /*= PERMANENT_GUI_ELEMENT*/)
    {
        m_frame = frame;
        m_texture = glText;
        m_destRect = destRect;
        m_color = color;
        //m_TTL = TTL;
        m_tickedTexture = TextureManager::getInstance().getTexture("Assets/RadioButton_Ticked.png");
        m_untickedTexture = TextureManager::getInstance().getTexture("Assets/RadioButton_Unticked.png");
    }

    bool RadioButton::hasCollision(const glm::vec2& posVec, int action /*= 0*/, bool isKeyHeld /*= false*/)
    {
        // catch to ensure disabled objects cannot be touched
        if(m_isDisabled || !m_isVisible) { return false; }

        if(isInside(posVec, m_centeredDestRect))
        {
            if(action == GUIMouse::LeftClick)
            {
                // let group functions handle it
                if(m_radioGroup) { m_radioGroup->tickButton(this); }
                else
                {
                    // if ungrouped, react accordingly
                    if(m_isTicked) { setTicked(false); }
                    else { setTicked(true); }
                }
            }

            return true;
        }

        // else
        return false;
    }

    bool RadioButton::update(float deltaTime /*= 0.0f*/)
    {
        // used for changes in position/size/etc
        if(m_needsUpdate)
        {
            m_needsUpdate = false;
            if(m_texture) { m_textureAddress = m_texture->getAddress(); }
            updateAlternateCoordinates(true);
            updateTextCentering(deltaTime);
        }

        if(m_isPermanent) { return false; }
        else { return m_TTL->isExpired(); }
    }

    void RadioButton::defaultTextures()
    {
		m_tickedTexture = TextureManager::getInstance().getTexture("Assets/RadioButton_Ticked.png");
        m_untickedTexture = TextureManager::getInstance().getTexture("Assets/RadioButton_Unticked.png");
        m_texture = (m_isTicked) ? m_tickedTexture : m_untickedTexture;
    }

    void RadioButton::joinRadioGroup(RadioGroup* rg)
    {
        // if true returned, it has been added to radio group
        if(rg->addButton(this)) { m_radioGroup = rg; }
    }

    void RadioButton::setTicked(bool ticked /*= false*/)
    {
        m_isTicked = ticked;
        if(m_isTicked) { m_texture = m_tickedTexture; }
        else { m_texture = m_untickedTexture; }
        m_needsUpdate = true;
    }

    void RadioButton::setTickedTexture(std::string path)
    {
        m_tickedTexture = TextureManager::getInstance().getTexture(path);
    }

    void RadioButton::setUntickedTexture(std::string path)
    {
        m_untickedTexture = TextureManager::getInstance().getTexture(path);
    }

    RadioButton* RadioGroup::getTicked()
    {
        for(unsigned int i = 0; i < m_radiobuttons.size(); i++)
        {
            if(m_radiobuttons[i]->isTicked()) { return m_radiobuttons[i]; }
        }

        // else
        return nullptr;
    }

    /// RadioGroup public functions ///////////////////////////////////////////

    RadioGroup::~RadioGroup()
    {
		// deletion falls to the frame to handle
		m_radiobuttons.clear();
    }

    bool RadioGroup::addButton(RadioButton* rb)
    {
        // if it's already got a group, don't bother
        if(rb->getRadioGroup() != nullptr) { return false; }

        // if it's already in here, don't bother
        for(unsigned int i = 0; i < m_radiobuttons.size(); i++)
        {
            if(m_radiobuttons[i] == rb) { return false; }
        }

        // else, add it to this group and return true
        m_radiobuttons.push_back(rb);
        return true;
    }

    void RadioGroup::tickButton(RadioButton* rb)
    {
        // loop through, unticking the others
        for(unsigned int i = 0; i < m_radiobuttons.size(); i++)
        {
            if(m_radiobuttons[i] == rb) { m_radiobuttons[i]->setTicked(true); }
            else { m_radiobuttons[i]->setTicked(false); }
        }
    }
}
