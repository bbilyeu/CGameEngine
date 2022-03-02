#ifndef GUIObject_H
#define GUIObject_H

#include "gui/gui_common.h"
#include "gui/Frame.h"
#include "common/Timer.h"
#include <vector>
#include <functional>
#include <string>

/*
	This is the base GUI object class, from which *ALL* GUI objects will use as a base.

	Ref:
		https://stackoverflow.com/questions/32270758/why-doesnt-c-support-strongly-typed-ellipsis
*/

/// \TODO: Replace TileSheet references
/// \TODO: Are the "get*CenteredXYZ actually correct?



namespace CGameEngine
{
	static bool isInside(glm::vec2 point, glm::vec4 bounds)
	{
//		glm::vec2 sc = convertV2ScrToWorld(point, e_screenWidth, e_screenHeight);
//		if((sc.x >= bounds.x && sc.x <= (bounds.x + bounds.z)) && (sc.y >= bounds.y && sc.y <= (bounds.y + bounds.w)))
//		{ return true; } else { return false; }
		if((point.x >= bounds.x && point.x <= (bounds.x + bounds.z)) && (point.y >= bounds.y && point.y <= (bounds.y + bounds.w)))
		{ return true; } else { return false; }
	}

	struct SubElement
    {
        SubElement() {}
        SubElement(glm::vec4 destrect, TextureAddress texAddr) : destRect(destrect), textureAddress(texAddr) {} // basic texture constructor
        SubElement(glm::vec4 destrect, std::string msg, bool isLableObj) : destRect(destrect), text(msg), isLabel(isLableObj) {} // basic label constructor
        SubElement(glm::vec4 destrect) : destRect(destrect) {}

        SubElement(const SubElement& dg) { copy(*this, dg); } // copy constructor
        SubElement(SubElement&& dg) { swap(*this, dg); } // move constructor
        SubElement& operator=(SubElement dg) { swap(*this, dg); return *this; } // copy/move assignment
        friend void copy(SubElement& dst, const SubElement& src)
        {
            if(&dst != &src)
            {
                dst.color = src.color;
                dst.textColor = src.textColor;
                dst.textCenter = src.textCenter;
                dst.destRect = src.destRect;
                dst.iconDestRect = src.iconDestRect;
                dst.iconUVRect = src.iconUVRect;
                dst.text = src.text;
                dst.textureAddress = src.textureAddress;
                dst.iconTextureAddress = src.iconTextureAddress;
                dst.isLabel = src.isLabel;
                dst.labelPosition = src.labelPosition;
                dst.iconPosition = src.iconPosition;
                dst.textJust = src.textJust;
            }
        }
        friend void swap(SubElement& dst, SubElement& src) { copy(dst, src); }

        void setIcon(glm::vec4 dr, glm::vec4 uv, TextureAddress iconTexAddr, char iconPos) { iconDestRect = dr; iconUVRect = uv; iconTextureAddress = iconTexAddr; iconPosition = iconPos; }
        void setText(std::string msg, glm::vec2 textcenter) { text = msg; textCenter = textcenter; }

        // member variables
        glm::ivec4 color = COLOR_NONE;
        glm::ivec4 textColor = COLOR_NONE;
        glm::vec2 textCenter = glm::vec2(0.0f);
        glm::vec4 destRect = glm::vec4(0.0f);
        glm::vec4 iconDestRect = glm::vec4(0.0f);
        glm::vec4 iconUVRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        glm::vec4 uvRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        std::string text = "NULL";
        Texture2D* texture = nullptr;
        TextureAddress textureAddress = {0,0};
        TextureAddress iconTextureAddress = {0,0};
        GUIGlyph* glyph = nullptr;
        GUIGlyph* iconGlyph = nullptr;
        bool isLabel = false;
        char labelPosition = GUIPositions::Top;
        char iconPosition = GUIPositions::Left;
        float angle = 0.0f;
        float iconAngle = 0.0f;
        int textJust = Justification::Center;
    };

	class GUIObject
    {
        public:
            GUIObject() : m_subElements(), m_onClickEvent() { m_inputManager = &InputManager::getInstance(); }
            virtual ~GUIObject();
            /// \TODO: Add pure virtual destructor and create necessary per-object destructors
            void onClickEvent() { if(m_onClickEvent) { m_onClickEvent(); } }
            virtual bool hasCollision(const glm::vec2& posVec, int action = 0, bool isKeyHeld = false);
            virtual bool update(float deltaTime = 0.0f) = 0;
            void forceUpdate() { m_needsUpdate = true; }
            virtual void defaultTextures() = 0;

            // getters
            Frame* getFrame() { return m_frame; }
            const glm::ivec4& getColor() const { return m_color; }
            const glm::ivec4& getTextColor() const { return m_textColor; }

            const glm::vec3 getTextScreenXYZ() const { return glm::vec3(m_centeredTextXY.x, m_centeredTextXY.y, 0.0f); }
            const glm::vec4& getScreenDestRect() const { return m_screenDestRect; }
            const glm::vec3 getScreenXYZ() const { return glm::vec3(m_screenDestRect.x, m_screenDestRect.y, 0.0f); }
            const glm::vec2 getScreenDimensions() const { return glm::vec2(m_screenDestRect.z, m_screenDestRect.w); }
            const glm::vec3 getIconScreenXYZ() const { return glm::vec3(m_iconDestRect.x, m_iconDestRect.y, 0.0f); }

            const glm::vec4& getDestRect() const { return m_destRect; }
            const glm::vec4& getCenteredDestRect() const { return m_centeredDestRect; }
            const glm::vec3 getCenteredXYZ() const { return glm::vec3(m_centeredDestRect.x, m_centeredDestRect.y, 0.0f); }
            const glm::vec2 getDimensions() const { return glm::vec2(m_destRect.z, m_destRect.w); }
            const glm::vec3 getXYZ() const { return glm::vec3(m_destRect.x, m_destRect.y, 0.0f); }
            const glm::vec4& getUVRect() const { return m_uvRect; }

            const glm::vec4& getIconDestRect() const { return m_iconDestRect; }
            const glm::vec2 getIconDimensions() const { return glm::vec2(m_iconDestRect.z, m_iconDestRect.w); }
            const glm::vec4& getIconUVRect() const { return m_iconUVRect; }

            const glm::vec2& getCenteredTextXY() const { return m_centeredTextXY; }
            const glm::vec4& getTextBoundsBox() const { return m_textBoundsBox; }

            Texture2D* getTexture() const { return m_texture; }
            const TextureAddress& getTextureAddress() const { return m_textureAddress; }
			const TextureAddress& getIconTextureAddress() const { return m_iconTextureAddress; }
			GUIGlyph* getGlyph() { return m_glyph; }
            GUIGlyph* getIconGlyph() { return m_iconGlyph; }
            const bool& hasCursor() const { return m_hasCursor; }
            const bool hasGlyph() const { return (m_glyph != nullptr); }
            const bool hasIconGlyph() const { return (m_iconGlyph != nullptr); }
            const bool hasText() const { if(m_text != "NULL") { return true; } /*else*/ return false; }
			const bool hasIconTexture() { return m_iconTextureAddress.exists(); }
            const bool hasTexture() { return m_textureAddress.exists(); }
            const bool hasSubElements() const { if(m_subElements.size() > 0) { return true; } /*else*/ return false; }
            const bool& isDisabled() const { return m_isDisabled; }
            const bool& isInEditMode() const { return m_isInEditMode; }
            const bool& isVisible() const { return m_isVisible; }
            const float& getAngle() const { return m_angle; }
            const float& getIconAngle() const { return m_iconAngle; }
            const float& getSubTextScaling() const { return m_subTextScaling; }
            const float& getTextScaling() const { return m_textScaling; }
            const int& getIconPosition() const { return m_iconPosition; }
            const int& getTextJustification() const { return m_textJustification; }
            const int& getType() const { return m_elementType; }
            const int& getValue() const { return m_value; }
            const std::string& getInternalName() const { return m_internalName; }
            const std::string& getText() const { return m_text; }
            const std::string getToolTipMessage() const { return m_toolTipMessage; }
            const std::vector<SubElement*> getSubElements() const { return m_subElements; }

            // setters
            void addIcon(); /// \Needed?
            void addIcon(glm::vec4& dr, glm::vec4& uv, TextureAddress texAddr, int pos = GUIPositions::Left); /// \FIXME
            void addIcons(TileSheet* ts, int iconPosition = GUIPositions::Left);
            void addIcons(std::string path, const glm::ivec2 columnsAndRows, int iconPosition = GUIPositions::Left); // add icons to elements
            void addLabel(std::string label, int position = GUIPositions::Top);
            void resetText() { m_text = std::to_string(m_value); }
            void resizeInFrame(glm::vec4& destRect);
            void setAngle(float f) { m_angle = f; }
            void setCursor(bool val = true) { m_hasCursor = val; m_needsUpdate = true; }
            void setColor(const glm::ivec4& color) { m_needsUpdate = true; m_color = color; }
            void setDestRect(const glm::vec4 destRect) { m_needsUpdate = true; m_destRect = destRect; }
            void setDisabled(bool val = true) { m_isDisabled = val; }
            void setFrame(Frame* frame) { m_frame = frame; }
            void setGlyph(GUIGlyph* g) { m_glyph = g; }
            void setIconAngle(float f) { m_iconAngle = f; }
            void setIconDestRect(glm::vec4& dr) { m_iconDestRect = dr; }
            void setIconPosition(int pos) { m_iconPosition = pos; }
            void setIconTextureAddress(TextureAddress addr) { m_iconTextureAddress = addr; m_needsUpdate = true; }
            void setIconGlyph(GUIGlyph* g) { m_iconGlyph = g; }
            void setIconUVRect(glm::vec4& uv) { m_iconUVRect = uv; }
            void setUseFrameScaling(bool val = true) { m_useFrameScaling = val; } // inverting bool
            void setLocation(glm::vec4& vecLoc) { m_needsUpdate = true; m_destRect.x = vecLoc.x; m_destRect.y = vecLoc.y; }
            void setOnClick(std::function<void()> clickEvent) { m_needsUpdate = true; m_onClickEvent = clickEvent; }
            void setSize(const glm::vec2& vecSize) { m_needsUpdate = true; m_destRect.z = vecSize.x; m_destRect.w = vecSize.y; }
            void setSubTextScaling(float scaling) { m_subTextScaling = scaling; }
            void setText(std::string text, glm::ivec4 color = COLOR_WHITE) { m_text = text; m_textColor = color; }
            void setTextBounds(float pctOffWidth, float pctOffHeight);
            void setTextJustification(int just);
            void setTextScaling(float scaling) { m_needsUpdate = true; m_textScaling = scaling; }
            void setTexture(const std::string& txPath);
            void setTexture(Texture2D* tx) { m_texture = tx; m_needsUpdate = true; }
			void setTextureAddress(TextureAddress addr) { m_textureAddress = addr; m_needsUpdate = true; }
            void setToolTipMessage(std::string msg) { m_toolTipMessage = msg; }
            void setVisible(bool val = true) { m_isVisible = val; }
            void trySetValue(std::string str);
            virtual void setValue(int val);

        protected:
			GUIObject(Frame* frame, std::string internalName, const glm::vec4 destRect) // building via JSON import
				: m_frame(frame), m_destRect(destRect), m_internalName(internalName) { m_inputManager = &InputManager::getInstance(); }
            glm::ivec4 m_color = COLOR_WHITE;
            glm::ivec4 m_textColor = COLOR_WHITE;
            Frame* m_frame = nullptr;
            Texture2D* m_texture = nullptr;
            TextureAddress m_textureAddress = {0,0};
            TextureAddress m_iconTextureAddress = {0,0};
            InputManager* m_inputManager = nullptr;
			GUIGlyph* m_glyph = nullptr;
			GUIGlyph* m_iconGlyph = nullptr;
            SubElement* m_label = nullptr;
            glm::vec2 m_centeredTextXY = glm::vec2(0.0f);
            glm::vec2 m_textBounds = glm::vec2(0.0f);
            glm::vec4 m_destRect = glm::vec4(0.0f);
            glm::vec4 m_iconDestRect = glm::vec4(0.0f);
            glm::vec4 m_iconUVRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            glm::vec4 m_centeredDestRect = glm::vec4(0.0f);
            glm::vec4 m_textBoundsBox = glm::vec4(0.0f);
            glm::vec4 m_uvRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
            glm::vec4 m_screenDestRect = glm::vec4(0.0f);
            TileSheet* m_iconSpriteSheet = nullptr;
            Timer* m_TTL = nullptr;
            void updateTextCentering(float deltatime = 0.0f, bool onlyLabel = false);
            void updateAlternateCoordinates(bool screen);
            bool m_hasCursor = false;
            bool m_useFrameScaling = true;
            bool m_isDisabled = false;
            bool m_isInEditMode = false;
            bool m_isPermanent = true; // does not expire (e.g. action bars)
            bool m_isVisible = true;
            bool m_needsUpdate = true;
            char iconPosition = GUIPositions::Left;
            float m_angle = 0.0f;
            float m_iconAngle = 0.0f;
            float m_subTextScaling = 1.0f;
            float m_textScaling = 1.0f;
            int m_iconPosition = GUIPositions::Left;
            int m_elementType = GUIElements::GUIObject;
            int m_textJustification = Justification::Center;
            int m_value = 0;
            std::string m_internalName = "";
            std::string m_text = "NULL";
            std::string m_toolTipMessage = "NULL";
            std::vector<SubElement*> m_subElements;
            std::function<void()> m_onClickEvent;
    };
}
#endif // GUIObject_H
