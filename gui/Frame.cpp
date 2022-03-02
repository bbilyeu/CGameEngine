#include "Frame.h"

#include "gui/gui_common.h"
#include "gui/GUI.h"

extern CGameEngine::GUI* e_gui;

namespace CGameEngine
{
	/// Frame public functions below //////////////////////////////////////////

    Frame::Frame(GUI* parent, std::string internalName, const glm::vec4 destRect) :
        m_parent(parent), m_destRect(destRect), m_internalName(internalName)
    {
        update();
    }

    Frame::~Frame()
    {
        for(auto& i : m_frameElements) { safeDelete(i.second); }
        m_frameElements.clear();
        m_parent = nullptr;
    }

    GUIObject* Frame::getCollisionObject(const glm::vec2& mouseCoords, const glm::vec2& prevMouseCoords, int action, bool isKeyHeld)
    {
        for(auto& i : m_frameElements)
        {
            if(i.second->hasCollision(mouseCoords, action, isKeyHeld)) { return i.second; }
        }

        // else
        if(m_isMoveable && isKeyHeld)
        {
            m_destRect.x = m_destRect.x + ((mouseCoords.x - prevMouseCoords.x) / e_screenWidth);
            m_destRect.y = m_destRect.y + ((mouseCoords.y - prevMouseCoords.y) / e_screenHeight);
            m_needsUpdate = true;
        }

        return nullptr;
    }

    GUIObject* Frame::getObject(std::string name)
    {
		if(m_frameElements.find(name) != m_frameElements.end())
		{
			return m_frameElements.at(name);
		}
		else { return nullptr; }
    }

    Dropdown* Frame::addDropdown(std::string name, glm::vec4 destRect)
    {
        Dropdown* dd = new Dropdown(this, name, destRect);
        if(dd)
        {
			m_frameElements.insert(std::make_pair(name, dd));
			return static_cast<Dropdown*>(m_frameElements.at(name));
		} else { return nullptr; }
    }

    PushButton* Frame::addPushButton(std::string name, glm::vec4 destRect)
    {
        PushButton* pb = new PushButton(this, name, destRect);
        if(pb)
        {
			Logger::getInstance().Log(Logs::DEBUG, Logs::GUI, "Frame::addPushButton()", "pushbutton mem addr [{}]", (void*)pb);
			m_frameElements.insert(std::make_pair(name, pb));
			return static_cast<PushButton*>(m_frameElements.at(name));
		} else { return nullptr; }
    }

    RadioButton* Frame::addRadioButton(std::string name, glm::vec4 destRect)
    {
        RadioButton* rb = new RadioButton(this, name, destRect);
        if(rb)
        {
			m_frameElements.insert(std::make_pair(name, rb));
			return static_cast<RadioButton*>(m_frameElements.at(name));
		} else { return nullptr; }
    }

    ScrollBox* Frame::addScrollBox(std::string name, glm::vec4 destRect)
    {
        ScrollBox* sb = new ScrollBox(this, name, destRect);
        if(sb)
        {
			m_frameElements.insert(std::make_pair(name, sb));
			return static_cast<ScrollBox*>(m_frameElements.at(name));
		} else { return nullptr; }
    }

    Slider* Frame::addSlider(std::string name, glm::vec4 destRect)
    {
        Slider* s = new Slider(this, name, destRect);
        if(s)
        {
			m_frameElements.insert(std::make_pair(name, s));
			return static_cast<Slider*>(m_frameElements.at(name));
		} else { return nullptr; }
    }

    TextObject* Frame::addTextObject(std::string name, std::string text, glm::vec4 destRect)
    {
        //m_parent->calcTextSize(destRect, text, textScaling);
        TextObject* to = new TextObject(this, name, destRect);
        if(to)
        {
			m_frameElements.insert(std::make_pair(name, to));
			return static_cast<TextObject*>(m_frameElements.at(name));
		} else { return nullptr; }
    }

    TickBox* Frame::addTickBox(std::string name, glm::vec4 destRect)
    {
        TickBox* tb = new TickBox(this, name, destRect);
        if(tb)
        {
			m_frameElements.insert(std::make_pair(name, tb));
			return static_cast<TickBox*>(m_frameElements.at(name));
		} else { return nullptr; }
    }

    bool Frame::hasCollision(const glm::vec2& mouseCoords)
    {
        return isInside(mouseCoords, m_centeredDestRect);
    }

    void Frame::update(float deltaTime /*= 0.0f*/)
    {
        if(m_needsUpdate)
        {
            // draw based on center, not a corner
            m_needsUpdate = false;

            if(m_texture) { m_textureAddress = m_texture->getAddress(); }

            // adjust size to that of original texture
			//Logger::getInstance().Log(Logs::DEBUG, "Frame::update()", "START: destRect ({}, {}, {}, {})", m_destRect.x, m_destRect.y, m_destRect.z, m_destRect.w);
            m_centeredDestRect = m_destRect;
            m_centeredDestRect.x -= (m_centeredDestRect.z / 2.0f);
            m_centeredDestRect.y -= (m_centeredDestRect.w / 2.0f);
            //Logger::getInstance().Log(Logs::DEBUG, "Frame::update()", "MID: centered destRect ({}, {}, {}, {})", m_centeredDestRect.x, m_centeredDestRect.y, m_centeredDestRect.z, m_centeredDestRect.w);

            // update coordinates for mouse interaction
            m_screenDestRect = m_destRect;
            m_screenDestRect.z *= e_screenWidth;
            m_screenDestRect.w *= e_screenHeight;
            m_screenDestRect.x = e_quadWidth + (m_screenDestRect.x * e_screenWidth) - (m_screenDestRect.z / 2.0f);
            m_screenDestRect.y = (m_screenDestRect.y * e_screenHeight) + e_quadHeight - (m_screenDestRect.w / 2.0f);
            //m_screenDestRect -= glm::vec4(e_screenWidth / 2.0f, e_screenHeight / 2.0f, 0.0f, 0.0f);
            //m_screenDestRect.x -= e_screenWidth / 2.0f;		// setting 0,0 to bottom left
            //m_screenDestRect.y -= e_screenHeight / 2.0f;

			//Logger::getInstance().Log(Logs::DEBUG, "Frame::update()", "END: m_screenDestRect destRect ({}, {}, {}, {})", m_screenDestRect.x, m_screenDestRect.y, m_screenDestRect.z, m_screenDestRect.w);
			Logger::getInstance().Log(Logs::DEBUG, Logs::GUI, "Frame::update()", "\033[1mscrPos [{}, {}], scrDim [{}, {}],  cnPos [{}, {}], cnDim [{}, {}]\033[0m",
				m_screenDestRect.x, m_screenDestRect.y, m_screenDestRect.w, m_screenDestRect.z, m_centeredDestRect.x, m_centeredDestRect.y, m_centeredDestRect.z, m_centeredDestRect.w);

            // frame has moved, update components
            for(auto& i : m_frameElements) { i.second->forceUpdate(); }
        }

        if(deltaTime > 0.0f)
        {
			std::queue<std::string> toDestroy;
            for(auto& i : m_frameElements)
            {
                bool destroy = i.second->update(deltaTime);
                if(destroy) { toDestroy.push(i.first); } // time for element to expire
            }

            while(!toDestroy.empty())
            {
				safeDelete(m_frameElements[toDestroy.front()]);
				m_frameElements.erase(toDestroy.front());
				toDestroy.pop();
			}
        }
    }

    void Frame::centerInFrame(glm::vec4& destRect)
    {
        // adjusts to a centered X and centered Y value
        destRect.x = m_centeredDestRect.x + (m_centeredDestRect.z / 2.0f) + ((m_centeredDestRect.z / 2.0f) * destRect.x);
        destRect.y = m_centeredDestRect.y + (m_centeredDestRect.w / 2.0f) + ((m_centeredDestRect.w / 2.0f) * destRect.y);
    }

    void Frame::resizeInFrame(glm::vec4& destRect)
    {
        centerInFrame(destRect);
        destRect.z = destRect.z * m_destRect.z;
        destRect.w = destRect.w * m_destRect.w;
    }
}
