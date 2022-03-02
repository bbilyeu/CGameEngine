#include "Window.h"


namespace CGameEngine
{
    Window::Window() {} // empty

    /**
     * Deconstructor, used to call SDL_DestroyWindow
     */
    Window::~Window() noexcept
    {
        SDL_DestroyWindow(m_sdlWindow);
        m_sdlWindow = nullptr;
    }

    /**
     * Function to create a window through SDL, setting needed openGL flags along the way
     *
     * @param windowName string showed at the top center of the created window (usually app/game name)
     * @param scrWidth screen width in pixels
     * @param scrHeight screen height in pixels
     * @param currentFlags setting flagsfrom enum WindowFlags, as needed
     */
    int Window::create(std::string windowName, int scrWidth, int scrHeight, unsigned int currentFlags)
    {
        Uint32 flags = SDL_WINDOW_OPENGL;
        m_screenHeight = scrHeight;
        m_screenWidth = scrWidth;
        if(currentFlags & INVISIBLE) { flags |= SDL_WINDOW_HIDDEN; } // hide window on start
        if(currentFlags & FULLSCREEN) { flags |= SDL_WINDOW_FULLSCREEN_DESKTOP; } // full screen to desktop resolution
        if(currentFlags & BORDERLESS) { flags |= SDL_WINDOW_BORDERLESS; }

        // force OpenGL 4.5 Core
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

        m_sdlWindow = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scrWidth, scrHeight, flags);
        if(m_sdlWindow == nullptr) { Logger::getInstance().Log(Logs::FATAL, Logs::Service, "Window::create()", "SDL Window could not be created! ({}, {}, {}, {})", windowName, scrWidth, scrHeight, currentFlags); }

        SDL_GLContext glContext = SDL_GL_CreateContext(m_sdlWindow); // initialize the opengl context for the _window
        if(glContext == nullptr) { Logger::getInstance().Log(Logs::FATAL, Logs::Service, "Window::create()", "SDL_GL context could not be created!"); }

		Logger::getInstance().Log(Logs::INFO, Logs::Service, "Window::create()", "OpenGL Version: {}, Window Size ({}, {})", glGetString(GL_VERSION), m_screenWidth, m_screenHeight);

        glClearColor(0.25f, 0.25f, 0.25f, 1.0f); // the clear-to color

        SDL_GL_SetSwapInterval(0); // set vsync

        // enable alpha blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);   // source, destination

        // free up memory
        //SDL_GL_DeleteContext(glContext);

        return 0;
    }

    /**
     * Swap to empty buffer for drawing, presenting current to user
     */
    void Window::swapBuffer()
    {
        SDL_GL_SwapWindow(m_sdlWindow); // using double buffer, swaps to empty buffer for drawing
    }
}
