#ifndef TIMER_H
#define TIMER_H

#include "common/types.h"
#include <chrono>
#include <string>

using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

namespace TimeUnits
{
    enum FORMS
    {
        NONE = 0,
        Microseconds,
        Milliseconds,
        Seconds,
        Minutes,
        Hours,
        END
    };
}

/// \TODO: Expand timer functionality to possibly use builtin chrono functions for efficiency
/// \TODO: Add remaining time functionality
/**
 * @file Timer.h
 * @brief Timer object that handles basic time related activities (countdowns, duration, etc)
 */
class Timer
{
    public:
        // handled via private function to avoid code duplication
        Timer(std::string name, uint32_t life, uint8_t type = TimeUnits::Milliseconds, bool noStart = false)
            : m_expired(noStart), m_name(name), m_timeUnits(type), m_life(life)  { if(!noStart) { setup(); } }
        virtual ~Timer() {}
        const bool& isExpired();
        const bool addTime(uint32_t val); // returned value is 'success of adding time' (fail = false)
        const uint8_t& getTimeUnits() const { return m_timeUnits; }
        const uint32_t& getLife() const { return m_life; }
        const uint32_t getTimeLeft() const;
        const std::string& getName() const { return m_name; }
        void setExpired() { m_expired = true; }
        void restart() { setup(); }
        void setLife(uint32_t val, bool noSetup = false) { m_life = val; if(!noSetup) { setup(); } } // set and reset
        void setTimeUnits(uint8_t val) { m_timeUnits = val; setup(); }

    private:
        void setup();
        void adjustTime(TimePoint& tp, uint32_t& value);
        bool m_expired = true;
        std::string m_name = ""; /**< assigned name  */
        uint8_t m_timeUnits = TimeUnits::NONE; /**< unit of time (µs, ms, s, m, h) */
        uint32_t m_life = 0;
        TimePoint m_end;
};

#endif // TIMER_H
