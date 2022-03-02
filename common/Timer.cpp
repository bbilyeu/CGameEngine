#include "common/Timer.h"

/**
 * return true/false if steady_clock::now() is greater than previous steady_clock value
 */
const bool& Timer::isExpired()
{
    if(!m_expired)
    {
        if(std::chrono::steady_clock::now() >= m_end) { m_expired = true; }
    }

    return m_expired;
}

/**
 * returned value is 'success of adding time' (fail = false)
 * 
 * @param val amount of time to be added (uses the configured time type)
 */
const bool Timer::addTime(uint32_t val)
{
	// not checking for expiration first as adjustTime() does this
	adjustTime(m_end, val);
	// inversion of m_expired (as m_expired == true means addTime failed)
	return (m_expired == true) ? false : true;
}

/**
 * get the current time left, converted into the configured time type
 * 
 * @return time left in configured time units (ms, s, m, etc)
 */
const uint32_t Timer::getTimeLeft() const
{
	uint32_t retVal = 0;

	switch(m_timeUnits)
    {
        case TimeUnits::Milliseconds:
            retVal = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(m_end - std::chrono::steady_clock::now()).count();
            break;
        case TimeUnits::Seconds:
            retVal = (uint32_t)std::chrono::duration_cast<std::chrono::seconds>(m_end - std::chrono::steady_clock::now()).count();
            break;
        case TimeUnits::Minutes:
            retVal = (uint32_t)std::chrono::duration_cast<std::chrono::minutes>(m_end - std::chrono::steady_clock::now()).count();
            break;
        case TimeUnits::Hours:
            retVal = (uint32_t)std::chrono::duration_cast<std::chrono::hours>(m_end - std::chrono::steady_clock::now()).count();
            break;
        default: // NONE or END
            break;
    }

    return retVal;
}

/// Timer private functions ///////////////////////////////////////////////////

/**
 * init() equivalent that sets m_expired to false and sets start time to now
 */
void Timer::setup()
{
    // clear previous value
    m_expired = false;

    // get start time
	m_end = std::chrono::steady_clock::now();
	adjustTime(m_end, m_life);
}

/**
 * adjust timer by adding X units of time to the passed point in time (TimePoint)
 * 
 * @param tp point in time to adjust
 * @param value X units to adjust (uses timer's configured time type)
 */
void Timer::adjustTime(TimePoint& tp, uint32_t& value)
{
	switch(m_timeUnits)
    {
        case TimeUnits::Milliseconds:
            tp += std::chrono::milliseconds(m_life);
            break;
        case TimeUnits::Seconds:
            tp += std::chrono::seconds(m_life);
            break;
        case TimeUnits::Minutes:
            tp += std::chrono::minutes(m_life);
            break;
        case TimeUnits::Hours:
            tp += std::chrono::hours(m_life);
            break;
        default: // NONE or END
            m_expired = true;
            break;
    }

    // check to ensure we didn't pull timer below now()
	isExpired();
}
