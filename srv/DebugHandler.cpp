#include "DebugHandler.h"
#include <iostream>
//#include <SDL2/SDL.h>
#include <cstdlib>

namespace CGameEngine
{
    // initialize debug level
    char DebugHandler::m_engineDebugLevel = 0;

    void debugInfo(char debugLevel, std::string debugString)
    {
        if(DebugHandler::getDebugLevel() && debugLevel <= DebugHandler::getDebugLevel()) { std::cout << debugString << std::endl; }
        //if(m_engineDebugLevel && level <= m_engineDebugLevel) { std::cout << debugString << std::endl; }
    }

    void fatalError(std::string errorString)
    {
        std::cout << errorString << std::endl;
        std::cout << "Press any key to quit...";
        int tmp;
        std::cin >> tmp;
        //SDL_Quit(); // shutdown the sdl engine
        exit(99); // actually exit program
    }
}
