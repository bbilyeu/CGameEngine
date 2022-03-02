#ifndef GUIRENDERER_H
#define GUIRENDERER_H

//#include <GL/glew.h>
//#include <GL/glcorearb.h>
#include <SDL2/SDL_video.h>
#include "gui/gui_common.h"
//#include "draw/Renderer2D.h"
#include "draw/FontManager.h"
#include "draw/DebugRenderer.h"

namespace CGameEngine
{
    class DebugRenderer;
    class DrawPath;
    class GLSLProgram;
    class Renderer2D;

//    class GUIRenderBatch
//    {
//        public:
//            GUIRenderBatch(GLuint OffSet, GLuint NumVertices, GLuint Texture, glm::vec4 ScissorBox, glm::vec4 pScissorBox) :
//                offset(OffSet), numVertices(NumVertices), texture(Texture), scissorBox(ScissorBox), parentScissorBox(pScissorBox) { }
//            ~GUIRenderBatch() = default;
//            GLuint offset = 0;
//            GLuint numVertices = 0;
//            GLuint texture = 0;
//            glm::vec4 scissorBox = glm::vec4(0.0f);
//            glm::vec4 parentScissorBox = glm::vec4(0.0f);
//    };

    class GUIRenderer
    {
        friend class GUI;

        public:
            // used for pointer initialization
            GUIRenderer() : m_scissorBox() {}
            // actual GUI initialization
            GUIRenderer(GUI* gui, bool drawDebug = false, std::string fontFile = "", char fontSize = 13);
            ~GUIRenderer() { dispose(); }
            void dispose();
            void renderGUI(const glm::mat4& cameraMatrix);
            void setDebug(bool val = true);
            //const int& getFontHeight() const { return m_spriteFont->getFontHeight(); }
            //const glm::vec2 getTextMeasurements(std::string str, float scaling) const { return m_spriteFont->measure(str.c_str(), scaling); }

        private:
			GLint prevProgram = 0, prevTexture = 0, prevActiveTexture = 0, prevABO = 0, prevEABO = 0,
				prevVAO = 0, prevBlendSource = 0, prevBlendDest = 0, prevBlendEqtRGB = 0,
				prevBlendEqtA = 0, prevViewport[4] = {0,0,0,0}, prevScissorBox[4] = {0,0,0,0};
            GLboolean prevEnableBlend = false, prevEnableCull = false, prevEnableDepth = false, prevEnableScissor = false;
            DrawPath* m_drawPath = nullptr;
            DrawPath* m_tempDrawPath = nullptr;
            DebugRenderer* m_debugRenderer = nullptr;
            GUI* m_gui = nullptr;
            GLSLProgram* m_glslProgram = nullptr;
            Renderer2D* m_r2d = nullptr;
            FontManager* m_fontManager = nullptr;
            const bool isInside(const glm::vec4& testee, const glm::vec4& container) const;
            //static bool compareFrontToBack(GUIRenderBatch* a, GUIRenderBatch* b);
            const glm::vec4 confine(const glm::vec4& confinee, const glm::vec4& container) const;

            void backupState();
            void restoreState();
            void buildVertices(int& cv, unsigned int gi);
            void drawGUI();
            void drawPopout(GUIObject* go, uint& offset);
            void drawSubElement(SubElement* se, GUIObject* co, uint& offset, glm::vec4& pScissorBox, glm::vec4& scissorBox);

            bool m_drawDebug = false;
            std::vector<glm::vec4> m_scissorBox;
            //std::vector<Glyph> m_glyphs;
            //std::vector<GUIRenderBatch*> m_guiRenderBatches;
    };
}

#endif // GUIRENDERER_H
