#ifndef SCREENLIST_H
#define SCREENLIST_H

#include <vector>

namespace CGameEngine
{
    class IMainGame;
    class IGameScreen;

    /**
     * @file ScreenList.h
     * @brief A ScreenList holds the IGameScreen objects and provides basic functionality for abstract control of them
     */
    class ScreenList
    {
        public:
            ScreenList(IMainGame* game);
            ~ScreenList();
            IGameScreen* getCurrentScreen();
            IGameScreen* moveNext();
            IGameScreen* movePrevious();
            void addScreen(IGameScreen* newScreen);
            void destroy();
            void setScreen(int nextScreen);

        protected:
            IMainGame* m_game = nullptr; /**< pointer to main game object */
            std::vector<IGameScreen*> m_screens; /**< list of IGameScreen objects */

            int m_currentScreenIndex = -1; /**< active IGameScreen based on m_screens (vector) */
    };
}
#endif // SCREENLIST_H
