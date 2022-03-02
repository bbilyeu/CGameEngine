#ifndef TIME_H
#define TIME_H

#include "common/types.h"
#include <chrono>
#include <string>

/*
    References:
        http://en.cppreference.com/w/cpp/chrono/duration/duration_cast
        http://www.informit.com/articles/article.aspx?p=1881386&seqNum=2
        http://en.cppreference.com/w/cpp/chrono

*/

using m_clock = std::chrono::system_clock;
using timepoint = std::chrono::time_point<m_clock>;

namespace CGameEngine
{
    class Time
    {
        public:
            static Time& getInstance()
            {
                static Time instance;
                return instance;
            }
            timepoint futureTP(uint64_t& MS);
            const uint32_t now() const;
            const uint64_t nowMS() const;
            const timepoint nowTP() const;
            std::string getTimestamp(bool precise = false);

        protected:
            Time() {}
            virtual ~Time() {}
            Time(const Time&) = delete;
            Time operator&(const Time&) = delete;
    };
}

#endif // TIME_H
