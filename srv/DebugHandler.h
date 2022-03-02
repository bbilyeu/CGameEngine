#ifndef DEBUGHANDLER_H
#define DEBUGHANDLER_H
#include <string>

namespace CGameEngine
{
    // adding extern allows any source file to utilize fatalError() as is without "Errors::fatalError()"

    enum debugLevels
    {
        DBGNONE = 0,
        DBGINFO = 1,
        DBGWARN = 2,
        DBRERR = 3,
        DBGVERBOSE = 4
    };


    extern void debugInfo(char debugLevel, std::string debugString);
    extern void fatalError(std::string errorString);

    class DebugHandler
    {
        public:
            static char getDebugLevel() { return m_engineDebugLevel; }
            static void setDebugLevel(char debugLevel) { m_engineDebugLevel = debugLevel; }

        private:
            static char m_engineDebugLevel;
    };
}
#endif
