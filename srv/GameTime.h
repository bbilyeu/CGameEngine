#ifndef GAMETIME_H
#define GAMETIME_H

#include <chrono>
#include <queue>

struct GameTimePoint
{
    GameTimePoint(uint8_t h = 0, uint8_t m = 0) :
        hour(h), minute(m) {}
    uint8_t hour = 0;
    uint8_t minute = 0;
};

using m_clock = std::chrono::_V2::steady_clock;

namespace CGameEngine
{
    /// \TODO: Review and improve
    /// \TODO: add minutes calculation (if relevant)
    /// \TODO: ERROR CHECK THIS BITCH
    // local time instance (i.e. zone time)
    class GameTime
    {
        public:
            GameTime(uint8_t gameHoursPerDay, uint32_t& msToGH, uint8_t inGameHour = 0);
            ~GameTime() {}
            GameTimePoint& getTime();

        protected:
            void fillHourQueue();
            GameTimePoint m_gameTime = GameTimePoint();
            uint8_t m_gameHoursPerDay = 0; // game hours to game day (i.e. 27h in a day)
            uint32_t m_msToGH = 0; // MS to game hours
            std::queue<std::chrono::time_point<m_clock>> m_hourQueue = std::queue<std::chrono::time_point<m_clock>>();
            std::chrono::time_point<m_clock> m_dayStart; // set at 12:00am equivalent
            std::chrono::time_point<m_clock> m_nextDay; // set at 11:59pm equivalent

        private:
            //static std::chrono::_V2::steady_clock m_clock;
    };

}
#endif // GAMETIME_H
