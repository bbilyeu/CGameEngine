#include "srv/Time.h"
#include <ctime> // localtime
#include <iomanip> // put_time
#include <sstream> // stringstream

namespace CGameEngine
{
    /*const float Time::getDeltaTime()
    {
        std::chrono::duration<float, std::milli> diff = std::chrono::steady_clock::now() - m_lastTick;
        m_lastTick = std::chrono::steady_clock::now();
        return diff.count(); // fractional milliseconds
    }*/

    timepoint Time::futureTP(uint64_t& MS)
    {
        return (m_clock::now() + std::chrono::milliseconds(MS));
    }

    const uint32_t Time::now() const
    {
        return (uint32_t)std::chrono::duration_cast<std::chrono::seconds>(m_clock::now().time_since_epoch()).count();
    }

    const uint64_t Time::nowMS() const
    {
        return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(m_clock::now().time_since_epoch()).count();
    }

    const timepoint Time::nowTP() const
    {
        return m_clock::now();
    }

    std::string Time::getTimestamp(bool precise /*= false*/)
    {
        auto tp = m_clock::now();
        auto t = m_clock::to_time_t(tp);
        std::stringstream ss(std::ios::out);
        ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
        if(precise)
        {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) % 1000;
            ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        }
        std::string tmp = ss.str();
        return tmp;
    }
}
