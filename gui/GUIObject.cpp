#include "GUIObject.h"

namespace CGameEngine
{
    GUIObject::~GUIObject()
    {
		for(auto& s : m_subElements)
		{
			safeDelete(s);
		}
		m_subElements.clear();
		safeDelete(m_TTL);
    }

    bool GUIObject::hasCollision(const glm::vec2& posVec, int action /*= 0*/, bool isKeyHeld /*= false*/)
    {
        // catch to ensure disabled objects cannot be touched
        if(m_isDisabled || !m_isVisible) { return false; }

        if(isInside(posVec, m_centeredDestRect))
        {
            if(action == GUIMouse::LeftClick) { onClickEvent(); }
            return true;
        }

        // else
        return false;
    }

    void GUIObject::addIcon()
    {
        if(m_iconSpriteSheet)
        {
            // create uniform box
            glm::vec4 iconDestRect = m_destRect;
            if(m_frame) { resizeInFrame(iconDestRect); }
            iconDestRect.z = std::min(nextBox(static_cast<int>(iconDestRect.z / 2.0f)), nextBox(static_cast<int>(iconDestRect.w / 2.0f)));
            iconDestRect.w = iconDestRect.z;
            iconDestRect.y = m_destRect.y + (m_destRect.w / 2.0f) - (iconDestRect.w / 2.0f);
            iconDestRect.x = m_destRect.x + (m_destRect.z / 2.0f) - (iconDestRect.z / 2.0f);

            // if box has text
            if(m_text != "NULL")
            {
                // icon positioning and text offsetting
                if(m_iconPosition == GUIPositions::Right)
                {
                    iconDestRect.x = m_centeredTextXY.x + e_guiFont->measure(m_text, m_textScaling).x;
                    m_centeredTextXY.x -= iconDestRect.z / 2.0f * 1.025f;
                }
                else // effectively all other positions become GUIPositions::Left
                {
                    iconDestRect.x = m_centeredTextXY.x - iconDestRect.z;
                    m_centeredTextXY.x += iconDestRect.z * 0.5f;
                }
            }

            m_iconDestRect = iconDestRect;
            m_iconUVRect = m_iconSpriteSheet->getUVs(0);
            m_iconTextureAddress = m_iconSpriteSheet->getTextureAddress();
        }
    }

    void GUIObject::addIcon(glm::vec4& dr, glm::vec4& uv, TextureAddress texAddr, int pos /*= GUIPositions::Left*/)
    {
        m_iconDestRect = dr;
        m_iconUVRect = uv;
        m_iconTextureAddress = texAddr;
        m_iconPosition = pos;
        m_needsUpdate = true;
    }

    void GUIObject::addIcons(TileSheet* ts, int iconPosition /*= GUIPositions::Left*/)
    {
        m_iconSpriteSheet = ts;
        m_iconPosition = iconPosition;
    }

    void GUIObject::addIcons(std::string path, const glm::ivec2 columnsAndRows, int iconPosition /*= GUIPositions::Left*/)
    {
        // delete old one, if exists
        //if(m_iconSpriteSheet) { delete m_iconSpriteSheet; }
        safeDelete(m_iconSpriteSheet);

        /*m_iconSpriteSheet = new TileSheet();
        m_iconSpriteSheet->init(TextureManager::getInstance().getTexture(path), columnsAndRows);
        m_iconPosition = iconPosition;*/
    }

    void GUIObject::addLabel(std::string label, int position /*= GUIPositions::Top*/)
    {
        // if label exists
        if(m_label)
        {
            // delete the old label
            for(unsigned int i = 0; i < m_subElements.size(); i++)
            {
                if(m_subElements[i] == m_label)
                {
                    m_subElements[i] = m_subElements.back();
                    m_subElements.pop_back();
                    safeDelete(m_label);
                    i=999;
                }
            }
        }

        // create the new label
        m_subElements.push_back(new SubElement());
        m_label = m_subElements.back();
        m_label->labelPosition = position;
        m_label->text = label;
        m_label->isLabel = true;

        m_needsUpdate = true;
    }

    void GUIObject::resizeInFrame(glm::vec4& destRect)
    {
        if(m_frame)
        {
			if(m_useFrameScaling) { m_frame->resizeInFrame(destRect); }
			else
            {
				// if not using frame scaling, use texture width/height
                m_frame->centerInFrame(destRect);
                destRect.z *= m_texture->getWidth();
                destRect.w *= m_texture->getHeight();
            }
        }
    }

    void GUIObject::setTextBounds(float pctOffWidth, float pctOffHeight)
    {
        m_textBounds[0] = pctOffWidth;
        m_textBounds[1] = pctOffHeight;
    }

    void GUIObject::setTextJustification(int just)
    {
        switch (just)
        {
            case 0:
                m_textJustification = Justification::Left;
                break;
            case 2:
                m_textJustification = Justification::Right;
                break;
            default:
                m_textJustification = Justification::Center;
                break;
        }
    }

    void GUIObject::setTexture(const std::string& txPath)
    {
		m_texture = TextureManager::getInstance().getTexture(txPath);
		m_needsUpdate = true;
    }

    void GUIObject::setValue(int val)
    {
        m_value = val;
        m_needsUpdate = true;
    }

    void GUIObject::trySetValue(std::string str)
    {
        // shave off trailing 'cursor'
        if(str.back() == '_') { str = str.substr(0, str.length() - 1); }

        // prevent crash from user being an idiot
        try { setValue(std::stoi(str)); }
        catch(...) { resetText(); }
    }

    void GUIObject::updateTextCentering(float deltatime /*= 0.0f*/, bool onlyLabel /*= false*/)
    {
        // if true, only center label (if exists)
        if(!onlyLabel)
        {
            // get text centering
			if(e_guiFont == nullptr) { Logger::getInstance().Log(Logs::FATAL, Logs::GUI, "GUIObject::updateTextCentering", "e_guiFont is null!"); }
            glm::vec2 tm = e_guiFont->measure(m_text, m_textScaling); // text measurements
            m_centeredTextXY.x = m_centeredDestRect.x + (m_centeredDestRect.z / 2.0f); // add half of the width to bottom-left x
            //m_centeredTextXY.y = m_centeredDestRect.y + ((m_centeredDestRect.w - (e_guiFont->fontHeight * m_textScaling) ) / 2.0f); // add half of height minus font height to bottom-left y
            m_centeredTextXY.y = m_centeredDestRect.y + (m_centeredDestRect.w / 2.0f) - (tm.y / 2.0f);

            // handle text box constraints
            if(m_textBounds != glm::vec2(0.0f))
            {
                float fracZ = m_centeredDestRect.z * m_textBounds[0];
                float fracW = m_centeredDestRect.w * m_textBounds[1];
                m_textBoundsBox.z = m_centeredDestRect.z - fracZ;
                m_textBoundsBox.w = m_centeredDestRect.w - fracW;
                m_textBoundsBox.x = (m_centeredDestRect.x + (fracZ / 2.0f));// - (m_textBoundsBox.z / 2.0f);
                m_textBoundsBox.y = (m_centeredDestRect.y + (fracW / 2.0f));// - (m_textBoundsBox.w / 2.0f);
            }

            // icon&text or icon w/o text
            if(m_iconTextureAddress.defined() && m_text != "NULL") // icon and text
            {
                // icon positioning and text offsetting
                if(m_iconPosition == GUIPositions::Right)
                {
                    m_iconDestRect.x = m_centeredTextXY.x + e_guiFont->measure(m_text, m_textScaling).x;
                    m_centeredTextXY.x -= m_iconDestRect.z / 2.0f * 1.025f;
                }
                else // effectively all other positions become GUIPositions::Left
                {
                    m_iconDestRect.x = m_centeredTextXY.x - m_iconDestRect.z;
                    m_centeredTextXY.x += m_iconDestRect.z * 0.5f;
                }
            }
            else if(m_iconTextureAddress.defined() && m_text == "NULL") // icon only, no text
            {
                m_iconDestRect.x = m_centeredDestRect.x + (m_centeredDestRect.z / 2.0f) - (m_iconDestRect.z / 2.0f);
                m_iconDestRect.y = m_centeredDestRect.y + (m_centeredDestRect.w / 2.0f) - (m_iconDestRect.w / 2.0f);
            }
        }

        if(m_label)
        {
            // starting coordinates
            glm::vec4 destRect = m_destRect;
            //std::cout<<"Parent Obj DR:\t"<<destRect.x<<"\t"<<destRect.y<<"\t"<<destRect.z<<"\t"<<destRect.w<<"\n";
            if(m_frame) { m_frame->centerInFrame(destRect); }
            //std::cout<<"Post CIF  DR:\t"<<destRect.x<<"\t"<<destRect.y<<"\t"<<destRect.z<<"\t"<<destRect.w<<"\n";
            glm::vec2 textCenter = glm::vec2(0.0f);

            // get text width and height
            glm::vec2 widthHeight = e_guiFont->measure(m_label->text, m_textScaling);
            destRect.z = widthHeight[0];
            destRect.w = widthHeight[1];
            destRect.x -= destRect.z / 2.0f;
            destRect.y -= destRect.w / 2.0f;
            m_label->destRect = destRect;

            // center text
            m_label->textCenter.x = m_label->destRect.x + (m_label->destRect.z / 2.0f);
            m_label->textCenter.y = m_label->destRect.y;

            //std::cout<<"Pre Switch DR:\t"<<m_label->destRect.x<<"\t"<<m_label->destRect.y<<"\t"<<m_label->destRect.z<<"\t"<<m_label->destRect.w<<"\n";

            // catch stupid values, adjust position, set text justification
            switch (m_label->labelPosition)
            {
                case GUIPositions::Left: // Positions:Left
                {
                    m_label->textJust = Justification::Right;
                    m_label->textCenter.x = m_centeredDestRect.x - (m_centeredDestRect.z * 0.05f); // buffer between object and label
                    m_label->destRect.x = m_label->textCenter.x - destRect.z;

                    break;
                }
                case GUIPositions::Right: // Positions:Right
                {
                    m_label->textJust = Justification::Left;
                    m_label->destRect.x = m_centeredDestRect.x + (m_centeredDestRect.z * 1.05f);
                    m_label->textCenter.x = m_label->destRect.x;
                    break;
                }
                case GUIPositions::Bottom: // Positions:Bottom
                {
                    m_label->destRect.y = m_centeredDestRect.y - (m_centeredDestRect.w * 0.05f) - e_guiFont->fontHeight; // bottom wall, 5% width diff
                    m_label->textCenter.y = m_label->destRect.y;
                    break;
                }
                default: // Positions:LTOP
                {
                    //std::cout<<"Default Case Pre:\t"<<m_label->destRect.y<<"\n";
                    m_label->destRect.y = m_centeredDestRect.y + (m_centeredDestRect.w * 1.025f); // top wall, 5% width add
                    //std::cout<<"Default Case Post:\t"<<m_label->destRect.y<<"\n";
                    m_label->textCenter.y = m_label->destRect.y;
                    break;
                }
            }

            //std::cout<<"Label DR:\t"<<m_label->destRect.x<<"\t"<<m_label->destRect.y<<"\t"<<m_label->destRect.z<<"\t"<<m_label->destRect.w<<"\n";
            //std::cout<<"Text Center:\t"<<m_label->textCenter.x<<"\t"<<m_label->textCenter.y<<"\n";
        }
    }

    void GUIObject::updateAlternateCoordinates(bool screen)
    {
		// update centered coordinates for collision
		m_centeredDestRect = m_destRect;
		glm::vec4 dr = m_centeredDestRect;

		if(m_frame)
		{
			// adjust size to relative portions of the frame
			resizeInFrame(dr);
			m_centeredDestRect.z = dr.z;
			m_centeredDestRect.w = dr.w;
		}
		else
		{
			// adjust size to that of original texture
			if(m_texture)
			{
				m_centeredDestRect.z = m_texture->getWidth() * dr.z;
				m_centeredDestRect.w = m_texture->getHeight() * dr.w;
			}
		}

		m_centeredDestRect.x = dr.x - m_centeredDestRect.z / 2.0f;
		m_centeredDestRect.y = dr.y - m_centeredDestRect.w / 2.0f;

		// update coordinates for mouse interaction
		if(screen)
		{
			if(m_frame)
			{
				glm::vec4 dr = m_destRect;
				resizeInFrame(dr);
				m_screenDestRect.z = dr.z * e_screenWidth;
				m_screenDestRect.w = dr.w * e_screenHeight;
			}
			else
			{
				m_screenDestRect.z = m_centeredDestRect.z * e_screenWidth;
				m_screenDestRect.w = m_centeredDestRect.w * e_screenHeight;
			}

            /// \TODO: Figure out wtf is going on with scaling/sizing of GUI elements
//			m_screenDestRect.z = m_centeredDestRect.z * e_screenWidth;
//			m_screenDestRect.w = m_centeredDestRect.w * e_screenHeight;

//            m_screenDestRect.x = e_quadWidth + (m_destRect.x * e_screenWidth) - (m_screenDestRect.z / 2.0f);
//            m_screenDestRect.y = e_quadHeight + (m_destRect.y * e_screenHeight) - (m_screenDestRect.w / 2.0f);
			m_screenDestRect.x = e_quadWidth + (m_centeredDestRect.x * e_screenWidth);
			m_screenDestRect.y = e_quadHeight + (m_centeredDestRect.y * e_screenHeight);
            Logger::getInstance().Log(Logs::DEBUG, Logs::GUI, "GUIObject::updateAlternateCoordinates()", "\033[1mscrPos [{}, {}], scrDim [{}, {}],  cnPos [{}, {}], cnDim [{}, {}]\033[0m",
				m_screenDestRect.x, m_screenDestRect.y, m_screenDestRect.z, m_screenDestRect.w, m_centeredDestRect.x, m_centeredDestRect.y, m_centeredDestRect.z, m_centeredDestRect.w);
		}
    }
}
