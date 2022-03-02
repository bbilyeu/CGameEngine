#include "IMainGame.h"
#include "ScreenList.h"
#include "IGameScreen.h"
//#include "draw/SpriteBatch.h"
#include "draw/DebugRenderer.h"
#include "common/types.h"


//CGameEngine::SpriteBatch* sb = new CGameEngine::SpriteBatch();
//CGameEngine::DebugRenderer* dbr = nullptr;

namespace CGameEngine
{
    /**
     * Initialize m_screenList and m_inputManager
     */
    IMainGame::IMainGame()
    {
        //m_screenList = std::make_unique<ScreenList>(this);
        m_screenList = new ScreenList(this);

        // if somehow the m_inputManager exists, we need to abort
        if(m_inputManager) { Logger::getInstance().Log(Logs::FATAL, Logs::Service, "IMainGame::IMainGame()", "'m_inputManager already exists!"); }
        else { m_inputManager = &InputManager::getInstance(); }
    }

    IMainGame::~IMainGame()
    {
        safeDelete(m_screenList); // covers screens too
    }

    /**
     * Handle cleanup as part of closing the application
     */
    void IMainGame::exitGame()
    {
		m_isRunning = false;
        m_currentScreen->onExit();
//        safeDelete(sb);
//        safeDelete(dbr);
    }

    /**
     * Trigger to relay input from SDL to the InputManager
     *
     * @param evnt SDL event passed
     */
    void IMainGame::onSDLEvent(SDL_Event& evnt)
    {
        switch (evnt.type) {
            case SDL_QUIT:
				m_isRunning = false;
                //exitGame();
                break;
            case SDL_MOUSEMOTION:
                m_inputManager->setMouseCoords(static_cast<float>(evnt.motion.x), static_cast<float>(evnt.motion.y));
                break;
            case SDL_KEYDOWN:
                m_inputManager->pressKey(evnt.key.keysym.sym);
                break;
            case SDL_KEYUP:
                m_inputManager->releaseKey(evnt.key.keysym.sym);
                break;
            case SDL_MOUSEBUTTONDOWN:
                m_inputManager->pressKey(evnt.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                m_inputManager->releaseKey(evnt.button.button);
                break;
        }
    }

    /**
     * First call to an IMainGame object, which includes the init() process
     */
    void IMainGame::run()
    {
        init();

        FPSLimiter limiter;
        limiter.setMaxFPS(60.0f);

        m_isRunning = true;
        while(m_isRunning)
        {
            limiter.beginFrame(); // begin FPS calculation

            m_inputManager->update(); // do input updating
            update(); // update all the things
            if(m_isRunning) // if game wasn't closed during update
            {
                draw(); // draw things
                m_fps = limiter.endFrame(); // get FPS value
                m_window.swapBuffer(); // swap draw buffer to visible buffer
            }
        }

        exitGame();
    }

    /// Protected functions below ///////////////////////////////////////////////

    /**
     * Starts the CGameEngine, the game application, and builds out structure to create game screens
     */
    bool IMainGame::init()
    {
        CGameEngine::init(); // init SDL
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        initSystems(); // load window
        onInit();
        addScreens();
        m_currentScreen = m_screenList->getCurrentScreen();
        m_currentScreen->onEntry();
        m_currentScreen->setRunning();
        return true;
    }

    /**
     * Creates the game window and runs glewInit()
     */
    void IMainGame::initSystems()
    {
        m_window.create("Default", 1440, 900, 0);

        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            // Problem: glewInit failed, something is seriously wrong.
            fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        }
        fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

        if (GLEW_ARB_debug_output) { printf("Supporting Arb output\n"); }
        if (GLEW_AMD_debug_output) { printf("Supporting AMD output\n"); }
        if (GLEW_KHR_debug) { printf("Supporting KHR output\n"); }
    }

    /**
     * Call to the current IGameScreen object's draw() function
     */
    void IMainGame::draw()
    {
		if(m_isRunning)
		{
			//glViewport(0, 0, m_window.getScreenWidth(), m_window.getScreenHeight());
			if(m_currentScreen && m_currentScreen->getState() == ScreenState::RUNNING)
			{
				m_currentScreen->draw();
			}
        }
    }

    /**
     * The game event loop function which handles screen state changes and current screen update() calls
     */
    void IMainGame::update()
    {
        if(m_currentScreen)
        {
            switch(m_currentScreen->getState())
            {
                case ScreenState::RUNNING:
                    m_currentScreen->update();
                    break;
                case ScreenState::CHANGE_NEXT:
                    m_currentScreen->onExit();
                    m_currentScreen = m_screenList->moveNext();
                    if(m_currentScreen)
                    {
                        m_currentScreen->setRunning();
                        m_currentScreen->onEntry();
                    }
                    break;
                case ScreenState::CHANGE_PREVIOUS:
                    m_currentScreen->onExit();
                    m_currentScreen = m_screenList->movePrevious();
                    if(m_currentScreen)
                    {
                        m_currentScreen->setRunning();
                        m_currentScreen->onEntry();
                    }
                    break;
                case ScreenState::EXIT_APPLICATION:
                    exitGame();
                    break;
                default:
                    break;
            }
        }
        else // no screen found!
        {
            exitGame();
        }
    }
}
