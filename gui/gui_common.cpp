#include "gui/gui_common.h"

namespace CGameEngine
{
	// shared 'globals'
	ActiveFont* e_guiFont = nullptr;
	int e_screenWidth = 0;
	int e_screenHeight = 0;
	int e_quadHeight = 0;
	int e_quadWidth = 0;
	glm::vec2 e_mouseVec = glm::vec2(0.0f);

	DropdownPopout* e_dropdownPopout = nullptr;
	GUI* e_gui = nullptr;
	GUIObject* e_hover = nullptr;
	GUIRenderer* e_renderer = nullptr;
}
