#ifndef IMAINGAME_H
#define IMAINGAME_H

#include "CGameEngine.h"
#include "srv/InputManager.h"
#include "draw/Framerate.h"
#include "Window.h"

namespace CGameEngine
{
    class ScreenList;
    class IGameScreen;

    /**
     * @file IMainGame.h
     * @brief IMainGame is the actual game process which maintains the event/draw loops, framerate calculation, and owns the input manager
     */
    class IMainGame
    {
        public:
            IMainGame();
            virtual ~IMainGame();

            const bool isRunning() const { return m_isRunning; }
            const float getFPS() const { return m_fps; }
            const float getMaxFPS() const { return m_MaxFPS; }

            void exitGame(); // exit the game
            void onSDLEvent(SDL_Event& evnt);
            void run(); /** run game and start main game loop */
            void setMaxFPS(float maxFPS) { m_MaxFPS = maxFPS; } /** set maximum FPS */

            virtual void onInit() = 0; // called through polymorphic windows/classes
            virtual void addScreens() = 0; // setup all the screens
            virtual void onExit() = 0; // handle all cleanup logic on exiting the game

        protected:
			InputManager* m_inputManager = nullptr; /**< Pointer to the InputManager singleton */
            IGameScreen* m_currentScreen = nullptr; /**< Pointer to the current game screen (e.g. main menu, level editor, etc) */
            ScreenList* m_screenList = nullptr; /**< List of IGameScreen objects */
            Window m_window; /**< The window object created and shown to the user */

            bool init();
            void initSystems();
            virtual void draw(); 
            virtual void update(); 

            bool m_isRunning = false;
            float m_fps = 0.0f; /**< FPS value from the FPS calculation */
            float m_MaxFPS = 60.0f; /**< Configured max FPS (frame limiting) */
            float m_newTicks = 0.0f; /**< 'Current' value to compare against m_prevTicks */
            float m_prevTicks = 0.0f; /**< Previous value from last fps calculation run. Compared to m_newTicks */

            const float MAX_DELTA_TIME = 1.0f; /**< Maximum time value for event and FPS calculation  */
            const int MAX_PHYSICS_STEPS = 6; /**< Maximum number of physics calculations per game loop iteration */
            const float MS_PER_SECOND = 1000.0f; 
            float DESIRED_FRAMETIME = MS_PER_SECOND / m_MaxFPS; /**< Calculated value for the expected time per frame */
    };
}
#endif // MAINGAME_H
