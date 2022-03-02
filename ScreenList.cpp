#include "ScreenList.h"
#include "IGameScreen.h"
#include "IMainGame.h"

namespace CGameEngine
{
    ScreenList::ScreenList(IMainGame* game) : m_game(game) {} // empty

    /**
     * Break down the game screens on the way to closing the application
     */
    ScreenList::~ScreenList()
    {
        destroy();
    }

    /**
     * Return the active or current IGameScreen object
     * 
     * @return pointer to the current IGameScreen
     */
    IGameScreen* ScreenList::getCurrentScreen()
    {
        // if no screens (somehow), return nullptr
        if(m_currentScreenIndex == SCREEN_INDEX_NO_SCREEN) { return nullptr; }

        // else, return screenIndex
        return m_screens[m_currentScreenIndex];
    }

    /**
     * Advance screen index and return 'next' IGameScreen object
     * 
     * @return pointer to the 'new' currentScreen IGameScreen
     */
    IGameScreen* ScreenList::moveNext()
    {
        IGameScreen* currentScreen = getCurrentScreen();
        if(currentScreen->getNextScreenIndex() != SCREEN_INDEX_NO_SCREEN)
        {
            // if next screen is set, go there
            m_currentScreenIndex = currentScreen->getNextScreenIndex();
        }

        return getCurrentScreen();
    }

    /**
     * Backtrack screen index and return 'previous' IGameScreen object
     * 
     * @return pointer to the 'old' currentScreen IGameScreen
     */
    IGameScreen* ScreenList::movePrevious()
    {
        IGameScreen* currentScreen = getCurrentScreen();
        if(currentScreen->getPreviousScreenIndex() != SCREEN_INDEX_NO_SCREEN)
        {
            // if next screen is set, go there
            m_currentScreenIndex = currentScreen->getPreviousScreenIndex();
        }

        return getCurrentScreen();
    }

    /**
     * Add a created IGameScreen object to the list and inform the IMainGame object
     * 
     * @param newScreen Created/configured IGameScreen object
     */
    void ScreenList::addScreen(IGameScreen* newScreen)
    {
        newScreen->m_screenIndex = m_screens.size();
        m_screens.push_back(newScreen);
        newScreen->build();
        newScreen->setParentGame(m_game);
    }

    /**
     * Called within the deconstructor, actually deletes each IGameScreen object it holds
     */
    void ScreenList::destroy()
    {
        for(unsigned int i = 0; i < m_screens.size(); i++)
        {
            if(m_screens[i] && m_screens[i] != nullptr) { delete m_screens[i]; } //[i]->destroy(); }
        }

        m_screens.clear();
        m_currentScreenIndex = SCREEN_INDEX_NO_SCREEN;
    }

    /**
     * Used to change a passed screen index (int) to the current screen
     * 
     * @param nextScreen index of the screen to make active
     */
    void ScreenList::setScreen(int nextScreen)
    {
        /// \TODO ADD VALIDATION TO ENSURE INDEX EXISTS!!!
        m_currentScreenIndex = nextScreen;
    }
}
