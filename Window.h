#ifndef WINDOW_H
#define WINDOW_H
//#include <GLES3/gl3.h>
//#include <GLFW/glfw3.h>
//#include <GL/gl.h>
//#include <GL/glew.h>
//#include <GL/glcorearb.h>
#include <SDL2/SDL.h>
#include <string>
#include <common/types.h>

/**
 * @file Window.h
 * @brief Header for the window object (visible window to users)
 */

namespace CGameEngine
{
    /**
     * Enum to hold the window flags (invis/hidden, borderless, or fullscreen)
     */
    enum WindowFlags
    {
        INVISIBLE = 0x1, /**< Window is minimized or hidden behind other applications */
        FULLSCREEN = 0x2, /**< Fullscreen in whatever OS context */
        BORDERLESS = 0x4 /**< Used primarily with Fullscreen Windowed */
    };

    /**
     * Class responsible for the 'active' Window presented to users
     */
    class Window
    {
        public:
            Window();
            ~Window();
            int create(std::string windowName, int scrWidth, int scrHeight, unsigned int currentFlags);
            const int& getScreenWidth() const { return m_screenWidth; }
            const int& getScreenHeight() const { return m_screenHeight; }
            SDL_Window* getSDLWindow() { return m_sdlWindow; }
            void swapBuffer();

        private:
            int m_screenWidth = 0; /** Screen width in pixels */
            int m_screenHeight = 0; /** Screen height in pixels */
            SDL_Window* m_sdlWindow = nullptr; /** Pointer to the SDL-created window object */
    };
}
#endif // WINDOW_H
