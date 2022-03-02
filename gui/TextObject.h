#ifndef TEXTOBJECT_H
#define TEXTOBJECT_H

#include "gui/GUIObject.h"

namespace CGameEngine
{
    class TextObject : public GUIObject
    {
        public:
            TextObject() {}
            TextObject(Frame* frame, std::string text, float textScaling, const glm::vec4 destRect, const glm::ivec4 color, float TTL = PERMANENT_GUI_ELEMENT);
            TextObject(Frame* frame, std::string internalName, glm::vec4 destRect) : GUIObject(frame, internalName, destRect) { update(); }
            bool update(float deltaTime = 0.0f) override;
            void defaultTextures() {}
    };
}

#endif // TEXTOBJECT_H
