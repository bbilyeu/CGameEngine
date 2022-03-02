#ifndef GUISHARED_H_INCLUDED
#define GUISHARED_H_INCLUDED

#include "common/types.h"
#include "common/util.h"
#include "common/glm_util.h" // includes glm.hpp
//#include "gui/GUIRenderer.h"
#include "srv/ConfigReader.h" // JSON r/w
#include "srv/InputManager.h"
//#include "srv/ResourceManager.h"
#include "draw/draw_common.h" // Glyph, DrawElementsIndirectCommand, RingBufferHead, vector
#include "draw/Texture2D.h"
#include "draw/TileSheet.h"
#include <unordered_map>
#include <string>
#include <functional> // passing functions as parameters


#define PERMANENT_GUI_ELEMENT -999
#define HALF_SECOND 500 // milliseconds
#define THREE_SECONDS 3000 // milliseconds

namespace CGameEngine
{
	class ContextMenu;
	class Dropdown;
	class DropdownPopout;
	class Frame;
	class GUI;
	class GUIObject;
    class GUIRenderer;
    class PushButton;
    class RadioButton;
    class RadioGroup;
    class ScrollBox;
    class Slider;
    class SubElement;
    class FontManager;
    class TextObject;
    class TickBox;
    class ToolTip;
    class Window;
    class Minimap;// : public GUIObject;
	class Wheel; // REALLY needed?

	struct GUIGlyph : Glyph
	{
		/* from Glyph
			TextureAddress textureAddress;
			std::vector<Vertex> vertices;
			glm::vec3 position;
		*/
		glm::vec4 scissorBox;
		glm::vec4 parentScissorBox;
	};

	// shared 'globals'
	extern ActiveFont* e_guiFont;
	extern int e_screenWidth;
	extern int e_screenHeight;
	extern int e_quadWidth;
	extern int e_quadHeight;
	extern glm::vec2 e_mouseVec;

	extern DropdownPopout* e_dropdownPopout;
	extern GUI* e_gui;
	extern GUIObject* e_hover;
	extern GUIRenderer* e_renderer;
}

namespace GUIElements
{
	enum FORMS
	{
		GUIObject = 0,
		Context,
		Dropdown,
		DropdownPopout,
		Minimap,
		PushButton,
		ScrollBox,
		Slider,
		TextBox,
		TextObject,
		ToolTip,
		END
	};
}

namespace GUIMouse
{
	enum FORMS
	{
		NONE = 0,
		MouseMove,
		LeftClick,
		RightClick,
		MWheelDown,
		MWheelUp,
		END
	};
}

namespace GUIPositions
{
	enum FORMS
	{
		NONE = 0,
		Left,
		Right,
		Top,
		Bottom,
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight,
		END
	};
}

/// These are defined and destroyed by the GUI class!
//extern CGameEngine::DropdownPopout* e_dropdownPopout;
//extern CGameEngine::GUIObject* e_hover;
//extern CGameEngine::GUIRenderer* e_renderer;
//extern int e_fontHeight;
//extern int e_guiFont->fontSize;
//extern int e_screenWidth; // m_window->getScreenWidth()
//extern int e_screenHeight; //  m_window->getScreenHeight()
//extern glm::vec2 e_mouseVec;

#endif // GUISHARED_H_INCLUDED
