#include "gui/GUIRenderer.h"

#include "gui/gui_common.h"
#include "gui/GUI.h"
#include "gui/Frame.h"
#include "CGameEngine.h"
//#include "draw/SpriteBatch.h"
#include "draw/Renderer2D.h"
#include "draw/FontManager.h"
#include <glm/gtc/matrix_transform.hpp> // orthographic view
#include "Window.h"
#include <algorithm>

#define DEBUG_LINE_WIDTH 1.5

extern CGameEngine::ActiveFont* e_guiFont;

namespace CGameEngine
{
    /// GUIRenderer public functions below ////////////////////////////////////

    GUIRenderer::GUIRenderer(GUI* gui, bool drawDebug /*= false,*/, std::string fontFile /*= ""*/, char fontSize /*= 13*/) :
        m_gui(gui), m_drawDebug(drawDebug), m_scissorBox()
    {
        // initialize fonts
        m_fontManager = &FontManager::getInstance();
        if(!e_guiFont) { e_guiFont = (fontFile == "") ? m_fontManager->initFont("guiFont", "Assets/AeroLight.ttf", 14) :  m_fontManager->initFont("guiFont", fontFile.c_str(), fontSize); }

        // initialize debug renderer (if needed)
        if(m_drawDebug)
        {
			if(!m_debugRenderer) { m_debugRenderer = new CGameEngine::DebugRenderer(); }
			m_debugRenderer->init();
		}
        m_r2d = &Renderer2D::getInstance();
        if(!m_r2d->init()) { Logger::getInstance().Log(Logs::FATAL, Logs::Generic, "GUIRenderer::GUIRenderer()", "Renderer2D did not finish init() successfully!"); }

        m_drawPath = new DrawPath();
        m_drawPath->pathType = DrawPathType::LONG;
        m_r2d->setupDrawPath(m_drawPath);
        m_tempDrawPath = new DrawPath();
        m_tempDrawPath->pathType = DrawPathType::FAST;
        m_r2d->setupDrawPath(m_tempDrawPath);

        m_glslProgram = new GLSLProgram();
		m_glslProgram->compileShadersFromSource(V450_VERT_SRC, V450_FRAG_SRC);
    }

    void GUIRenderer::dispose()
    {
		//if(m_spriteFont) { m_spriteFont->dispose(); }
		m_scissorBox.clear();
		//m_glyphs.clear();
		if(m_drawPath) { m_drawPath->clear(true); }
		if(m_tempDrawPath) { m_tempDrawPath->clear(true); }
		//safeDelete(m_spriteFont);
    }

    void GUIRenderer::renderGUI(const glm::mat4& cameraMatrix)
    {
        if(!m_gui || m_gui->m_isRunning == false || SDL_GetWindowFlags(m_gui->m_window->getSDLWindow()) == (SDL_WindowFlags::SDL_WINDOW_HIDDEN | SDL_WindowFlags::SDL_WINDOW_MINIMIZED))
        {
            // window isn't visible, don't draw
			Logger::getInstance().Log(Logs::VERBOSE, Logs::GUI, "GUIRenderer::renderGUI()", "Window is hidden or minimized.");
            return;
        }

        // create glyphs
        drawGUI();

        // prepare draw paths
        m_r2d->setCameraMatrix(cameraMatrix, m_glslProgram->getProgramID());
        m_r2d->renderPrep(m_drawPath);
        //m_r2d->renderPrep(m_tempDrawPath);

        // actually render the data
        m_r2d->renderDrawPath(m_drawPath, m_glslProgram->getProgramID());
        //m_r2d->renderDrawPath(m_tempDrawPath);

        // debug render box borders
        if(m_drawDebug)
        {
            m_debugRenderer->end();
            m_debugRenderer->render(cameraMatrix, DEBUG_LINE_WIDTH);
        }
    }

	void GUIRenderer::setDebug(bool val /*= true*/)
	{
		m_drawDebug = val;
        if(m_drawDebug)
        {
			if(!m_debugRenderer) { m_debugRenderer = new CGameEngine::DebugRenderer(); }
			m_debugRenderer->init();
		}
	}

    /// GUIRenderer private functions below ///////////////////////////////////

    void GUIRenderer::backupState()
    {
        // Backup GL state
        glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);
        glGetIntegerv(GL_BLEND_SRC, &prevBlendSource);
        glGetIntegerv(GL_BLEND_DST, &prevBlendDest);
        glGetIntegerv(GL_BLEND_EQUATION_RGB, &prevBlendEqtRGB);
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &prevBlendEqtA);
        glGetIntegerv(GL_SCISSOR_BOX, prevScissorBox);
        prevEnableBlend = glIsEnabled(GL_BLEND);
        prevEnableCull = glIsEnabled(GL_CULL_FACE);
        prevEnableDepth = glIsEnabled(GL_DEPTH_TEST);
        prevEnableScissor = glIsEnabled(GL_SCISSOR_TEST);
    }

    void GUIRenderer::restoreState()
    {
        // Restore GL state
        if(prevProgram) glUseProgram(prevProgram);
        glBlendEquationSeparate(prevBlendEqtRGB, prevBlendEqtA);
        glBlendFunc(prevBlendSource, prevBlendDest);
        if (prevEnableBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (prevEnableCull) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        if (prevEnableDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (prevEnableScissor) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        glScissor(prevScissorBox[0], prevScissorBox[1], (GLsizei)prevScissorBox[2], (GLsizei)prevScissorBox[3]);
    }

//    void GUIRenderer::buildVertices(int& cv, unsigned int gi)
//    {
//        m_drawPath->vertices[cv++] = m_glyphs[gi].topLeft;
//        m_drawPath->vertices[cv++] = m_glyphs[gi].bottomLeft;
//        m_drawPath->vertices[cv++] = m_glyphs[gi].bottomRight;
//        m_drawPath->vertices[cv++] = m_glyphs[gi].bottomRight;
//        m_drawPath->vertices[cv++] = m_glyphs[gi].topRight;
//        m_drawPath->vertices[cv++] = m_glyphs[gi].topLeft;
//    }

    void GUIRenderer::drawGUI()
    {
        unsigned int offset = 0;
        unsigned int tmpOffset = 0;
        glm::vec4 scissorBox = glm::vec4(0.0f);
        glm::vec4 pScissorBox = glm::vec4(0.0f);

		// grid
        if(m_drawDebug && m_debugRenderer)
        {
			glm::vec2 pos = glm::vec2(0.0f);
			for(float x = -0.5f; x < 0.5f; x+=0.025f)
			{
				for(float y = -0.5f; y < 0.5f; y+=0.025f)
				{
					pos.x = x;
					pos.y = y;
					m_debugRenderer->drawCircle(pos, COLOR_GREY, 0.001f);
				}
			}
        }

        /// FRAMES ///
        for(auto& guiFr : m_gui->m_frames)
        {
            Frame* fr = guiFr.second; // get current frame
            pScissorBox = glm::vec4(fr->getScreenDestRect().x, fr->getScreenDestRect().y, fr->getScreenDestRect().z, fr->getScreenDestRect().w);
            if(fr->m_texture)
            {
                /*Logger::getInstance().Log(Logs::DEBUG, "GUIRenderer::drawGUI()", "CDR ({}, {}, {}, {})\tUVR ({}, {}, {}, {})",
											fr->m_centeredDestRect.x, fr->m_centeredDestRect.y, fr->m_centeredDestRect.z, fr->m_centeredDestRect.w,
											fr->m_uvRect.x, fr->m_uvRect.y, fr->m_uvRect.z, fr->m_uvRect.w);*/

				if(fr->m_glyph != nullptr) { fr->m_glyph->position = fr->getScreenXYZ(); }
				else
				{
					Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "GUIRenderer::drawGUI()", "\033[1mFRAME\033[0m\t\t txAddr [{}, {}], pos [{}, {}, 0], dim [{}, {}], uvRect [{}, {}, {}, {}], angle [{}]",
						fr->m_textureAddress.containerHandle, fr->m_textureAddress.sliceNumber, fr->getScreenDestRect().x, fr->getScreenDestRect().y, fr->getScreenDimensions().x, fr->getScreenDimensions().y, fr->m_uvRect.x, fr->m_uvRect.y, fr->m_uvRect.z, fr->m_uvRect.w, fr->m_angle);
					GUIGlyph* gl = (GUIGlyph*)m_r2d->targetedDraw(m_drawPath, fr->m_textureAddress, fr->getScreenXYZ(), fr->getScreenDimensions(), fr->m_uvRect, fr->m_angle, true);
					//GUIGlyph* gl = (GUIGlyph*)m_r2d->targetedDraw(m_drawPath, fr->m_textureAddress, glm::vec3(1.0f), glm::vec3(150.0f), fr->m_uvRect, fr->m_angle);
					fr->m_glyph = gl;
				}
            }
            if(m_drawDebug) { m_debugRenderer->drawBox(fr->getScreenDestRect(), COLOR_RED); } // draw frame box

            /// FRAME ELEMENTS ///
            for(auto& frEl : fr->m_frameElements)
            {
                // get current guiobject (co = currentObject)
                GUIObject* co = frEl.second;

                // do not draw or render invisible objects
                if(co->isVisible())
                {
                    // create scissorbox
                    scissorBox = glm::vec4(co->getCenteredDestRect().x, co->getCenteredDestRect().y, co->getCenteredDestRect().z, co->getCenteredDestRect().w);

                    // debug drawing
                    if(m_drawDebug) { m_debugRenderer->drawBox(co->getScreenDestRect(), COLOR_YELLOW); m_debugRenderer->drawBox(glm::vec4(co->getScreenDestRect().x, co->getScreenDestRect().y, 2.0f, 2.0f), COLOR_BLACK); }

                    // add to renderbatches and glyphs
                    if(co->hasTexture())
                    {
						GUIGlyph* gl = nullptr;
						if(co->hasGlyph()) { gl = co->getGlyph(); gl->position = co->getScreenXYZ(); }
                        else
                        {
							Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "GUIRenderer::drawGUI()", "ELEMENT\t txAddr [{}, {}], pos [{}, {}, 0], dim [{}, {}], uvRect [{}, {}, {}, {}], angle [{}]",
								co->getTextureAddress().containerHandle, co->getTextureAddress().sliceNumber, co->getScreenDestRect().x, co->getScreenDestRect().y, co->getScreenDimensions().x,
								co->getScreenDimensions().y, co->getUVRect().x, co->getUVRect().y, co->getUVRect().z, co->getUVRect().w, co->getAngle());
							gl = (GUIGlyph*)m_r2d->targetedDraw(m_drawPath, co->getTextureAddress(), co->getScreenXYZ(), co->getScreenDimensions(), co->getUVRect(), co->getAngle(), true);
							co->setGlyph(gl);
                        }

//                        if(gl != nullptr)
//                        {
//							Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "GUIRenderer::drawGUI()", "\033[94msizeof GUIGlyph [{}], Glyph [{}], *gl [{}]", sizeof(GUIGlyph), sizeof(Glyph), sizeof(*gl));
//							gl->parentScissorBox = pScissorBox;
//							gl->scissorBox = scissorBox;
//						}
                    }

                    // if it has text, add that to renderbatches and glyphs
                    if(co->hasText())
                    {
                        // set appropriate scissorbox
                        if(co->getTextBoundsBox() != glm::vec4(0.0f)) { scissorBox = co->getTextBoundsBox(); }
                        else { scissorBox = glm::vec4(co->getCenteredDestRect().x, co->getCenteredDestRect().y, co->getCenteredDestRect().z, co->getCenteredDestRect().w); }

                        // debug drawing
                        //if(m_drawDebug) { m_debugRenderer->drawBox(scissorBox, COLOR_YELLOW); m_debugRenderer->drawBox(glm::vec4(scissorBox.x, scissorBox.y, 2.0f, 2.0f), COLOR_BLACK); }
                        if(m_drawDebug) { m_debugRenderer->drawBox(co->getTextBoundsBox(), COLOR_YELLOW); m_debugRenderer->drawBox(glm::vec4(co->getTextScreenXYZ().x, co->getTextScreenXYZ().y, 2.0f, 2.0f), COLOR_YELLOW); }

                        m_fontManager->targetedDraw(m_drawPath, e_guiFont, co->getText().c_str(), co->getCenteredTextXY(), co->getTextScaling(), 0.0f, co->getTextColor(), co->getTextJustification());
                    }

                    // render icon
                    if(co->hasIconTexture())
                    {
                        /// \TODO : Is this actually going to work or will I need 'centered' position?
//                        if(co->hasIconGlyph()) { co->getIconGlyph()->position = co->getIconCenteredXYZ(); }
//                        else
//                        {
//							GUIGlyph* gl = (GUIGlyph*)m_r2d->targetedDraw(m_drawPath, co->getIconTextureAddress(), co->getIconCenteredXYZ(), co->getIconDimensions(), co->getIconUVRect(), co->getIconAngle());
//							co->setIconGlyph(gl);
//                        }
						//co->getIconGlyph()->parentScissorBox = pScissorBox;
						//co->getIconGlyph()->scissorBox = scissorBox;
                    }

                    // render subelements
                    if(co->hasSubElements())
                    {
                        std::vector<SubElement*> subElements = co->getSubElements();
                        for(unsigned int s = 0; s < subElements.size(); s++) { drawSubElement(subElements[s], co, offset, pScissorBox, subElements[s]->destRect); }
                    }
                }

                // cleanup
                scissorBox = glm::vec4(0.0f);
                tmpOffset = 0;
                co = nullptr;
            }

            // cleanup
            fr = nullptr;
            pScissorBox = glm::vec4(0.0f);
            scissorBox = glm::vec4(0.0f);
        }

        /// POPOUT, CONTEXT-MENU, TOOLTOP ///
        if(e_dropdownPopout && e_dropdownPopout != nullptr) { drawPopout(static_cast<GUIObject*>(e_dropdownPopout), offset); }
        if(m_gui->m_contextMenu) { drawPopout(static_cast<GUIObject*>(m_gui->m_contextMenu), offset); }
        if(m_gui->m_toolTip && m_gui->m_toolTip->isVisible())
        {
            // get tooltip
            ToolTip* tt = m_gui->m_toolTip;

            // create scissorbox
            scissorBox = glm::vec4(tt->getCenteredDestRect().x, tt->getCenteredDestRect().y, tt->getCenteredDestRect().z, tt->getCenteredDestRect().w);
            pScissorBox = scissorBox;

            // background
            m_r2d->targetedDraw(m_tempDrawPath, tt->getTextureAddress(), tt->getXYZ(), tt->getDimensions(), tt->getUVRect(), tt->getAngle());

            // tooltip text
            //m_fontManager->targetedDraw(m_tempDrawPath, e_guiFont, tt->getText().c_str(), tt->getCenteredTextXY(), tt->getTextScaling(), 0.0f, tt->getTextColor(), tt->getTextJustification());

            // debug box drawing and cleanup
            if(m_drawDebug) { m_debugRenderer->drawBox(tt->getCenteredDestRect(), COLOR_PURPLE); }
        }
    }

    void GUIRenderer::drawPopout(GUIObject* go, uint& offset)
    {
        //DropdownPopout* go = m_gui->m_dropdownPopout;
        glm::vec4 pScissorBox = glm::vec4(0.0f);
        glm::vec4 scissorBox = glm::vec4(0.0f);

        // create scissorbox (out of centered dest rect)
        pScissorBox = glm::vec4(go->getCenteredDestRect().x, go->getCenteredDestRect().y, go->getCenteredDestRect().z, go->getCenteredDestRect().w);

        // draw background
        if(go->hasTexture())
		{
			m_r2d->targetedDraw(m_tempDrawPath, go->getTextureAddress(), go->getXYZ(), go->getDimensions(), go->getUVRect(), go->getAngle());
		}

        std::vector<SubElement*> boxes = go->getSubElements();
        for(unsigned int i = 0; i < boxes.size(); i++) { drawSubElement(boxes[i], go, offset, pScissorBox, boxes[i]->destRect); }

       /*if(m_drawDebug) { m_debugRenderer->drawBox(go->getCenteredDestRect(), COLOR_YELLOW); m_debugRenderer->drawBox(glm::vec4(go->getCenteredDestRect().x, go->getCenteredDestRect().y, 2.0f, 2.0f), COLOR_BLACK); }*/
    }

    void GUIRenderer::drawSubElement(SubElement* se, GUIObject* co, unsigned int& offset, glm::vec4& pScissorBox, glm::vec4& scissorBox)
    {
        unsigned int tmpOffset = 0;

        // render texture/background/etc
        if(se->textureAddress.exists())
        {
            if(se->glyph) { se->glyph->position = glm::vec3(se->destRect.x, se->destRect.y, 0.0f); }
            else
            {
				GUIGlyph* gl = (GUIGlyph*)m_r2d->targetedDraw(m_drawPath, se->textureAddress, glm::vec3(se->destRect.x, se->destRect.y, 0.0f), glm::vec2(se->destRect.z, se->destRect.w), se->uvRect, se->angle);
				se->glyph = gl;
            }
            se->glyph->parentScissorBox = pScissorBox;
            se->glyph->scissorBox = scissorBox;
        }

        // render text
        if(se->text != "NULL")
        {
            m_fontManager->targetedDraw(m_drawPath, e_guiFont, se->text.c_str(), se->textCenter, co->getTextScaling(), 0.0f, se->textColor, se->textJust);
        }

        // render icon
        if(se->iconTextureAddress.exists())
        {
			if(se->iconGlyph) { se->iconGlyph->position = glm::vec3(se->iconDestRect.x, se->iconDestRect.y, 0.0f); }
            else
            {
				GUIGlyph* gl = (GUIGlyph*)m_r2d->targetedDraw(m_drawPath, se->iconTextureAddress, glm::vec3(se->iconDestRect.x, se->iconDestRect.y, 0.0f), glm::vec2(se->iconDestRect.z, se->iconDestRect.w), se->iconUVRect, se->iconAngle);
				se->iconGlyph = gl;
            }
            se->iconGlyph->parentScissorBox = pScissorBox;
            se->iconGlyph->scissorBox = scissorBox;
        }

        // debug borders and origin point
        if(m_drawDebug) { m_debugRenderer->drawBox(se->destRect, COLOR_YELLOW); m_debugRenderer->drawBox(glm::vec4(se->destRect.x, se->destRect.y, 2.0f, 2.0f), COLOR_BLACK); }
    }

    const bool GUIRenderer::isInside(const glm::vec4& testee, const glm::vec4& container) const
    {
        if( (testee.x >= container.x && (testee.x + testee.z) <= (container.x + container.z) ) && // x test
            (testee.y >= container.y && (testee.y + testee.w) <= (container.y + container.w) ) ) // y test
        { return true; } // testee is inside
        else { return false; } // testee is NOT inside
    }

    const glm::vec4 GUIRenderer::confine(const glm::vec4& confinee, const glm::vec4& container) const
    {
        glm::vec4 retVal = confinee;
        //std::cout << "Confinee:\t " << confinee.x << "\t" << confinee.y << "\t" << confinee.z << "\t" << confinee.w << "\n";

        // if starting X is outside container
        if(confinee.x < container.x || confinee.x > (container.x + container.z)) { retVal.x = container.x; }
        // if starting Y is outside container
        if(confinee.y < container.y || confinee.y > (container.y + container.w)) { retVal.y = container.y; }
        // if X + width exceeds container
        if((confinee.x + confinee.z) > (container.x + container.z)) { retVal.z = (container.x + container.z) - confinee.x; }
        // if Y + height exceeds container
        if((confinee.y + confinee.w) > (container.y + container.w)) { retVal.w = (container.y + container.w) - confinee.y; }

        //std::cout << "RetVal:\t\t " << retVal.x << "\t" << retVal.y << "\t" << retVal.z << "\t" << retVal.w << "\n";
        return retVal;
    }

//    bool GUIRenderer::compareFrontToBack(GUIRenderBatch* a, GUIRenderBatch* b)
//    {
//        if(a->parentScissorBox.x > b->parentScissorBox.x)
//        {
//            if(a->parentScissorBox.y > b->parentScissorBox.y)
//            {
//                return true;
//            }
//        }
//        return false;
//    }
}
