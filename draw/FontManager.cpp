#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include "draw/FontManager.h"
#include "draw/Renderer2D.h"
#include "draw/TextureManager.h"

/// \TODO: Remove closestPow2 and use one from util.h

/// LEGEND
// gi = glyph index
// si = string index (character)
// ce = character end
// cs = character start

int closestPow2(int i)
{
    i--;
    int pi = 1;
    while (i > 0)
    {
        i >>= 1;
        pi <<= 1;
    }
    return pi;
}

#define MAX_TEXTURE_RES 4096

namespace CGameEngine
{
    /**
     * @brief convert font to drawable glyphs
     *
     * @param identifier name (usually font name)
     * @param font raw data of the font
     * @param size size in pixels
     * @param charStart first printable character
     * @param charEnd last printable character
     * @return ActiveFont* pointer to the newly created ActiveFont object
     */
    ActiveFont* FontManager::initFont(std::string identifier, const char* font, int size, char charStart /*= FIRST_PRINTABLE_CHAR*/, char charEnd /*= LAST_PRINTABLE_CHAR*/)
    {
		//Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "FontManager::initFont()", "identifier '{}', font '{}', size '{}', charStart '{}', charEnd '{}'", identifier, font, size, charStart, charEnd);
		ActiveFont* newFont = new ActiveFont();
        TTF_Font* f = TTF_OpenFont(font, size);
        if(!f || f == nullptr || f == NULL)
        {
			Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "FontManager::initFont()", "Failed to open TTF font {}.", font);
        }

        newFont->fontSize = size; 					// ex: 14pt
        newFont->fontHeight = TTF_FontHeight(f);
        newFont->startOffset = charStart;			// first usable character
        newFont->length = charEnd - charStart - 1;	// length (last usable minus first usable)

        int padding = size / 8;

        // First measure all the regions
        std::vector<glm::ivec4> glyphRects;
        int i = 0, advance;
        for (char c = charStart; c <= charEnd; c++)
        {
			glyphRects.push_back(glm::ivec4(0));
            TTF_GlyphMetrics(f, c, &glyphRects[i].x, &glyphRects[i].z, &glyphRects[i].y, &glyphRects[i].w, &advance);
            glyphRects[i].z -= glyphRects[i].x;
            glyphRects[i].x = 0;
            glyphRects[i].w -= glyphRects[i].y;
            glyphRects[i].y = 0;
            i++;
        }

        // Find best partitioning of glyphs
        int rows = 1, width, height, bestWidth = 0, bestHeight = 0, area = MAX_TEXTURE_RES * MAX_TEXTURE_RES, bestRows = 0;
        bool firstPass = true;
        std::vector<int>* bestPartition = nullptr;
        while (rows <= newFont->length)
        {
            height = rows * (padding + newFont->fontHeight) + padding;
            auto gr = createRows(&glyphRects, newFont->length, rows, padding, width);
            Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "FontManager::initFont()", "\033[92mSTAGE 1\033[0m: row [{}], width [{}], height [{}], bestWidth [{}], bestHeight [{}], area [{}], bestRows [{}], MAX_TEXTURE_RES [{}], newFont->length [{}]", rows, width, height, bestWidth, bestHeight, area, bestRows, MAX_TEXTURE_RES, newFont->length);

            // Desire a power of 2 texture
			width = closestPow2(width);
			height = closestPow2(height);

            // A texture must be feasible
            if (width > MAX_TEXTURE_RES || height > MAX_TEXTURE_RES)
            {
                rows++;
                safeDeleteArray(gr);
                continue;
            }

            Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "FontManager::initFont()", "\033[92mSTAGE 2\033[0m: row [{}], width [{}], height [{}], bestWidth [{}], bestHeight [{}], area [{}], bestRows [{}], MAX_TEXTURE_RES [{}], newFont->length [{}]", rows, width, height, bestWidth, bestHeight, area, bestRows, MAX_TEXTURE_RES, newFont->length);

            // Check for minimal area
            if (area >= width * height)
            {
                safeDeleteArray(bestPartition);
                bestPartition = gr;
                bestWidth = width;
                bestHeight = height;
                bestRows = rows;
                area = bestWidth * bestHeight;
                rows++;
            }
            else { safeDeleteArray(gr); break; }

            Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "FontManager::initFont()", "\033[93mSTAGE 3\033[0m: row [{}], width [{}], height [{}], bestWidth [{}], bestHeight [{}], area [{}], bestRows [{}], MAX_TEXTURE_RES [{}], newFont->length [{}]", rows-1, width, height, bestWidth, bestHeight, area, bestRows, MAX_TEXTURE_RES, newFont->length);
        }

        Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "FontManager::initFont()", "\033[1mEND STAGE\033[0m: bestWidth [{}], bestHeight [{}]", bestWidth, bestWidth);

        // Can a bitmap font be made?
        if (!bestPartition)
        {
			Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "FontManager::initFont()", "Failed to Map TTF font '{}' to texture! (Try lowering resolution?)", font);
        }

        // Create the texture
        //glGenTextures(1, &m_texID);
        //glBindTexture(GL_TEXTURE_2D, m_texID);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, bestWidth, bestHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        //glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
        if(!m_txContainer)
        {
			m_txContainer = TextureManager::getInstance().newTextureContainer(1, GL_RGBA32F, bestWidth, bestHeight, newFont->length * 2);
        }

		Texture2D* texture = new Texture2D(m_txContainer, m_txContainer->virtualAllocate());
		newFont->textureAddress = texture->getAddress();

        // Now draw all the glyphs
        SDL_Color fontColor = { 255, 255, 255, 255 };
        int yOffset = padding;
        for (int rowIndex = 0; rowIndex < bestRows; rowIndex++)
        {
            int xOffset = padding;
            for (int charIndex = 0; charIndex < bestPartition[rowIndex].size(); charIndex++)
            {
                int glyphIndex = bestPartition[rowIndex][charIndex];
                SDL_Surface* glyphSurface = TTF_RenderGlyph_Blended(f, static_cast<char>(charStart + glyphIndex), fontColor);
                if(glyphSurface == nullptr)
                {
					Logger::getInstance().Log(Logs::FATAL, Logs::Drawing, "FontManager::initFont()", "Failed to render blended glyph. charIndex [{}] of [{}]", charIndex, bestPartition[rowIndex].size());
                }

                // Pre-multiplication occurs here
                unsigned char* surfacePixels = (unsigned char*)glyphSurface->pixels;
                int charPixels = glyphSurface->w * glyphSurface->h * 4;
                for (int i = 0; i < charPixels; i += 4)
                {
                    float a = surfacePixels[i + 3] / 255.0f;
                    surfacePixels[i] *= a;
                    surfacePixels[i + 1] = surfacePixels[i];
                    surfacePixels[i + 2] = surfacePixels[i];
                }

                // Save glyph image and update coordinates
                //glTexSubImage2D(GL_TEXTURE_2D, 0, lx, bestHeight - ly - 1 - glyphSurface->h, glyphSurface->w, glyphSurface->h, GL_RGBA, GL_UNSIGNED_BYTE, glyphSurface->pixels);
                //glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
                Texture2D* tmpTx = new Texture2D(m_txContainer, m_txContainer->virtualAllocate());
                /// \TODO: Will 'GL_UNSIGNED_BYTE'` work for 'imageSize'?
                tmpTx->setTexSubImage2D(0, xOffset, (bestHeight - yOffset - 1 - glyphSurface->h), glyphSurface->w, glyphSurface->h, GL_RGBA, GL_DONT_CARE, glyphSurface->pixels);
                glyphRects[glyphIndex].x = xOffset;
                glyphRects[glyphIndex].y = yOffset;
                glyphRects[glyphIndex].z = glyphSurface->w;
                glyphRects[glyphIndex].w = glyphSurface->h;

                SDL_FreeSurface(glyphSurface);
                glyphSurface = nullptr;

                xOffset += glyphRects[glyphIndex].z + padding;
            }
            yOffset += newFont->fontHeight + padding;
        }

        //Logger::getInstance().Log(Logs::DEBUG, Logs::Drawing, "FontManager::initFont()", "Post surface generation iterations");

        // Draw the unsupported glyph
        int rs = padding - 1;
        //if( rs<= 1 ) { rs = 2; }
        int* pureWhiteSquare = new int[rs * rs];
        memset(pureWhiteSquare, 0xffffffff, rs * rs * sizeof(int));
        //glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, rs, rs, GL_RGBA, GL_UNSIGNED_BYTE, pureWhiteSquare);
        Texture2D* tmpTx = new Texture2D(m_txContainer, m_txContainer->virtualAllocate());
        /// \TODO: Which setTexSubImage2D?!
        tmpTx->setTexSubImage2D(0, 0, 0, rs, rs, GL_RGBA, GL_UNSIGNED_BYTE, pureWhiteSquare);
        safeDeleteArray(pureWhiteSquare); //delete[] pureWhiteSquare;
        pureWhiteSquare = nullptr;

        /// \TODO: Determine if needed
        // Set some texture parameters
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        // //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

        /// \TODO: Read https://open.gl/textures and decide on best practice for fonts

		// create glyphs
        for (int i = 0; i < newFont->length; i++)
        {
			newFont->cGlyphs.push_back(new CharGlyph());
            newFont->cGlyphs.back()->character = (char)(charStart + i);
            newFont->cGlyphs.back()->dimensions = glm::vec2(glyphRects[i].z, glyphRects[i].w);
            newFont->cGlyphs.back()->uvRect = glm::vec4(
                (float)glyphRects[i].x / (float)bestWidth,
                (float)glyphRects[i].y / (float)bestHeight,
                (float)glyphRects[i].z / (float)bestWidth,
                (float)glyphRects[i].w / (float)bestHeight);
        }
        newFont->cGlyphs.push_back(new CharGlyph());
        newFont->cGlyphs[newFont->length]->character = ' ';
        newFont->cGlyphs[newFont->length]->dimensions = newFont->cGlyphs[0]->dimensions;
        newFont->cGlyphs[newFont->length]->uvRect = glm::vec4(0, 0, (float)rs / (float)bestWidth, (float)rs / (float)bestHeight);

        //glGenerateMipmap(GL_TEXTURE_2D);	// should be unneeded

        //glBindTexture(GL_TEXTURE_2D, 0);
        //safeDeleteArray(glyphRects);
        safeDeleteArray(bestPartition);
        if(f) { TTF_CloseFont(f); }
        f = nullptr;

        m_fonts.insert(std::make_pair(identifier, newFont));
        return newFont;
    }

    /**
     * @brief used to draw a string (e.g. floating damage numbers, on-screen message) using default draw path
     *
     * @param font ActiveFont object (a.k.a which font to use)
     * @param s char* string of the characters to draw
     * @param position starting position from which the string is drawn
     * @param scaling size of the font (%)
     * @param depth closeness to screen
     * @param tint RGBA color value (0-255)
     * @param just text justification
     */
    void FontManager::draw(ActiveFont* font, const char* s, glm::vec2 position, float scaling, float depth, glm::ivec4 tint, int just /* = Justification::Left */)
    {
		if(!font) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "FontManager::targetedDraw", "Passed ActiveFont pointer is null!"); return; }
		else if(!s) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "FontManager::targetedDraw", "Passed char string pointer is null!"); return; }

		doDraw(nullptr, font, s, position, scaling, depth, tint, just);
    }

    /**
     * @brief used to draw a string (e.g. floating damage numbers, on-screen message) using provided draw path
     *
     * @param p drawpath to use
     * @param font ActiveFont object (a.k.a which font to use)
     * @param s char* string of the characters to draw
     * @param position starting position from which the string is drawn
     * @param scaling size of the font (%)
     * @param depth closeness to screen
     * @param tint RGBA color value (0-255)
     * @param just text justification
     */
    void FontManager::targetedDraw(DrawPath* p, ActiveFont* font, const char* s, glm::vec2 position, float scaling, float depth, glm::ivec4 tint, int just /* = Justification::Left */)
    {
		if(!p) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "FontManager::targetedDraw", "Passed DrawPath pointer is null!"); return; }
		else if(!font) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "FontManager::targetedDraw", "Passed ActiveFont pointer is null!"); return; }
		else if(!s) { Logger::getInstance().Log(Logs::CRIT, Logs::Drawing, "FontManager::targetedDraw", "Passed char string pointer is null!"); return; }

        doDraw(p, font, s, position, scaling, depth, tint, just);
    }

//    void FontManager::drawToGlyph(std::vector<Glyph>& glyphVec, const char* s, glm::vec2 position, float scaling, float depth, glm::ivec4 tint, int just /* = Justification::Center */, bool addCursor /*= false*/, bool invertY /*= true*/)
//    {
//        glm::vec2 tp = position;
//
//        // Apply justification
//        if (just == Justification::Center) { tp.x -= measure(s, (scaling/2.0f)).x; }
//        else if (just == Justification::Right) { tp.x -= measure(s, scaling).x; }
//
//        for (int si = 0; s[si] != 0; si++)
//        {
//            char c = s[si];
//            if (s[si] == '\n')
//            {
//                if(invertY) { tp.y -= static_cast<float>(m_fontHeight) * scaling; }
//                else { tp.y += static_cast<float>(m_fontHeight) * scaling; }
//                tp.x = position.x;
//            }
//            else
//            {
//                // Check for correct glyph
//                int gi = c - m_startOffset;
//                if (gi < 0 || gi >= newFont->length) { gi = newFont->length; }
//                glm::vec4 destRect(tp, newFont->cGlyphs[gi].size * scaling);
//                glyphVec.emplace_back(destRect, newFont->cGlyphs[gi].uvRect, m_texID, depth, tint);
//                tp.x += newFont->cGlyphs[gi].size.x * scaling;
//            }
//        }
//
//        // add blinking cursor to end, without affecting justification
//        if(addCursor)
//        {
//            char c = '_';
//            int gi = c - m_startOffset;
//            if (gi < 0 || gi >= newFont->length) { gi = newFont->length; }
//            glm::vec4 destRect(tp, newFont->cGlyphs[gi].size * scaling);
//            glyphVec.emplace_back(destRect, newFont->cGlyphs[gi].uvRect, m_texID, depth, tint);
//            tp.x += newFont->cGlyphs[gi].size.x * scaling;
//        }
//    }

    /// Private functions below ///////////////////////////////////////////////

    FontManager::FontManager() { if (!TTF_WasInit()) { TTF_Init(); } }

    FontManager::~FontManager()
    {
        // clear up all character glyphs across all fonts
        for(auto f : m_fonts)
        {
            for(auto g : f.second->cGlyphs) { safeDelete(g); }
            f.second->cGlyphs.clear();
            safeDelete(f.second);
        }
        m_fonts.clear();
//        while(!m_fonts.empty())
//        {
//			ActiveFont* f = m_fonts.end()->second;
//			while(!f->cGlyphs.empty()) { safeDelete(f->cGlyphs.back()); f->cGlyphs.pop_back(); }
//			safeDelete(f);
//			m_fonts.erase(m_fonts.end());
//        }
        safeDelete(m_txContainer); // handles font GL data destruction
        TTF_Quit();
    }

    void FontManager::doDraw(DrawPath* p, ActiveFont* font, const char* s, glm::vec2 position, float scaling, float depth, glm::ivec4 tint, int just /* = Justification::Left */)
    {
        glm::vec2 tp = position;
        Renderer2D* r2D = &Renderer2D::getInstance();
        // Apply justification
        if (just == Justification::Center) { tp.x -= font->measure(s, (scaling/2.0f)).x; }
        else if (just == Justification::Right) { tp.x -= font->measure(s, scaling).x; }
        for (int stringIndex = 0; s[stringIndex] != 0; stringIndex++)
        {
            char c = s[stringIndex];
            if (s[stringIndex] == '\n')
            {
                tp.y += font->fontHeight * scaling;
                tp.x = position.x;
            }
            else
            {
                // Check for correct glyph
                int glyphIndex = c - font->startOffset;
                if (glyphIndex < 0 || glyphIndex >= font->length) { glyphIndex = font->length; }
                //glm::vec4 destRect(tp, font->cGlyphs[glyphIndex].size * scaling);
                //batch.draw(destRect, font->cGlyphs[glyphIndex].uvRect, (GLuint)m_texID, depth, tint);
//                if(p) { r2D->targetedDraw(p, font->textureAddress, glm::vec3(tp, depth), glm::vec2(font->cGlyphs[glyphIndex]->dimensions * scaling), font->cGlyphs[glyphIndex]->uvRect, 0.0f); }
//                else { r2D->draw(font->textureAddress, glm::vec3(tp, depth), glm::vec2(font->cGlyphs[glyphIndex]->dimensions * scaling), font->cGlyphs[glyphIndex]->uvRect, 0.0f); }
				r2D->draw(font->textureAddress, glm::vec3(tp, depth), glm::vec2(font->cGlyphs[glyphIndex]->dimensions * scaling), font->cGlyphs[glyphIndex]->uvRect, 0.0f); /// DEBUG
                tp.x += font->cGlyphs[glyphIndex]->dimensions.x * scaling;
            }
        }
    }

    /**
     * @brief create rows of characters as part of font initialization
     *
     * @param rects
     * @param rectsLength
     * @param r
     * @param padding
     * @param w
     * @return std::vector<int>*
     */
    std::vector<int>* FontManager::createRows(std::vector<glm::ivec4>* rects, int rectsLength, int r, int padding, int& w)
    {
        // Blank initialize
        std::vector<int>* l = new std::vector<int>[r]();
        int* cw = new int[r]();
        for (int i = 0; i < r; i++) { cw[i] = padding; }

        // Loop through all glyphs
        for (int i = 0; i < rectsLength; i++)
        {
            // Find row for placement
            int ri = 0;
            for (int rii = 1; rii < r; rii++)
            if (cw[rii] < cw[ri]) { ri = rii; }

            // Add width to that row
            cw[ri] += (*rects)[i].z + padding;

            // Add glyph to the row list
            l[ri].push_back(i);
        }

        // Find the max width
        w = 0;
        for (int i = 0; i < r; i++) { if (cw[i] > w) w = cw[i]; }

        // clean up memory leak
        safeDeleteArray(cw); //delete[] cw;

        // return vector
        return l;
    }
}
