#include "GameTime.h"

namespace CGameEngine
{
    /// GameTime public functions ///////////////////////////////////////////

    GameTime::GameTime(uint8_t gameHoursPerDay, uint32_t& msToGH, uint8_t inGameHour /*= 0*/) :
        m_gameHoursPerDay(gameHoursPerDay), m_msToGH(msToGH)
    {
        if(inGameHour > 0)
        {
            uint64_t offset = m_msToGH * inGameHour;
            m_dayStart = m_clock::now() - std::chrono::milliseconds(offset);

            uint8_t remainingHours = gameHoursPerDay - inGameHour;
            m_nextDay = m_clock::now() + std::chrono::milliseconds(remainingHours * m_msToGH);
            m_gameTime.hour = inGameHour;
        }
        else
        {
            m_dayStart = m_clock::now();
            m_nextDay = m_clock::now() + std::chrono::milliseconds(m_gameHoursPerDay * m_msToGH);
        }

        fillHourQueue();
    }

    GameTimePoint& GameTime::getTime()
    {
        // if no windows left and next day started
        if(m_hourQueue.empty() && m_nextDay >= m_dayStart)
        {
            m_dayStart = m_nextDay;
            m_nextDay = m_clock::now() + std::chrono::milliseconds(m_gameHoursPerDay * m_msToGH);
            m_gameTime.hour = 0;

        }
        // if the hour has "ended", move to the next one
        else if(m_clock::now() > m_hourQueue.front())
        {
            m_gameTime.hour++;
            m_hourQueue.pop();
        }

        return m_gameTime;
    }

    void GameTime::fillHourQueue()
    {
        // get the MS threshold for each new "hour"
        //      (i.e. 1am gametime = ?? ms)
        for(uint8_t i = 0; i < m_gameHoursPerDay; i++)
        {
            m_hourQueue.push( m_dayStart + std::chrono::milliseconds(m_gameHoursPerDay * i) );
        }

        for(uint8_t i = 0; i < m_gameTime.hour; i++) { m_hourQueue.pop(); }
    }
}
