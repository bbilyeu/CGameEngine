#ifndef IGAMESCREEN_H
#define IGAMESCREEN_H
#include "draw/Framerate.h" // FPSLimiter

#define SCREEN_INDEX_NO_SCREEN -1

/**
 * @file IGameScreen.h
 * @brief Interface class for game screen objects
 */
namespace CGameEngine
{
    /**
     * Enum built to track the state of each 'GameScreen'
     */
    enum class ScreenState
    {
        NONE, /**< 0 or unsent */
        RUNNING, /**< Most often used state */
        EXIT_APPLICATION, /**< Indicate that it is time to exit */
        CHANGE_NEXT, /**< Change to the screen ID set in m_nextScreen */
        CHANGE_PREVIOUS /**< Change to the screen ID set in m_prevScreen */
    };

    class IMainGame;

    /**
     * GameScreen Interface object to handle core functionality
     */
    class IGameScreen
    {
        friend class ScreenList;

        public:
            IGameScreen() {}
            virtual ~IGameScreen() {}

            /// pure virtual functions
            virtual void build() = 0; // called at screen creation (constructor)
            virtual void destroy() = 0; // called at screen destruction (destructor)

            // called when a screen enters/exits "focus"
            virtual void onEntry() = 0;
            virtual void onExit() = 0;

            // called in the main game loop
            virtual void draw() = 0;
            virtual void update() = 0;

            // screen index data
            virtual int getNextScreenIndex() const = 0;
            virtual int getPreviousScreenIndex() const = 0;

            // getters
            int getScreenIndex() const { return m_screenIndex; }
            ScreenState getState() const { return m_currentState; }

            // setters
            void setParentGame(IMainGame* game) { m_game = game; }
            void setRunning() { m_currentState = ScreenState::RUNNING; }

        protected:
            FPSLimiter* m_fpsLimiter = nullptr;
            ScreenState m_currentState = ScreenState::NONE;
            IMainGame* m_game = nullptr;

            bool m_drawDebug = false; /**< determines if DebugRender is live and debug rendering is active */
            int m_screenIndex = -1; /**< screen index used by the ScreenList class */
    };
}
#endif // IGAMESCREEN_H
