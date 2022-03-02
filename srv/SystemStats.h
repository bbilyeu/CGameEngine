#ifndef SYSTEMSTATS_H
#define SYSTEMSTATS_H

#include "common/types.h"
#include "common/Timer.h"

#if PLATFORM == PLATFORM_WINDOWS // Windows
    #include <Windows.h>
    #include <psapi.h>
#else // Linux
    //#include <unistd.h>
    //#include <sys/resource.h>
    #include <sys/sysinfo.h>
#endif

#define LINUX_SYSINFO_LOADS_SCALE 65536

// Ref: https://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-run-time-in-c
// Ref: http://man7.org/linux/man-pages/man2/sysinfo.2.html
// Ref: https://www.linuxquestions.org/questions/programming-9/%27load-average%27-return-values-from-sysinfo-309720/
// GOLD REF: https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process

/// \TODO: Test heavily on Windows system to ensure values returned are desired.

namespace CGameEngine
{
    class SystemStats
    {
        public:
            static SystemStats& getInstance()
            {
                static SystemStats sys;
                return sys;
            }
            void update();
            void setDelay(int val) { m_updateTimer->setLife(val); } // change timer delay

            const float& getLoadAverageHourly() const { return m_loadAverageHourly; }
            const float& getLoadAverage() const { return m_loadAverage; }
            const float& getMemoryAverage() const { return m_memoryAverage; }
            const float& getSwapUsage() const { return m_swapUsage; }
            const uint32_t& getUptime() const { return m_uptime; } // Seconds
            const uint32_t& getMemoryMax() const { return m_memoryMax; } // MB
            const uint32_t& getSwapMax() const { return m_swapMax; } // MB
            const unsigned& getCPUCores() const { return m_cores; }

        private:
            SystemStats();
            virtual ~SystemStats();

            // transient stats
            float m_loadAverageHourly = 0.0f;
            float m_loadAverage = 0.0f;
            float m_loadArray[4] = {0};
            uint8_t m_LA = 0;

            float m_memoryAverage = 0.0f;
            float m_memoryArray[20] = {0};
            uint8_t m_MA = 0;

            float m_swapUsage = 0.0f;
            uint32_t m_uptime = 0; // Seconds

            // 'static' stats
            uint32_t m_memoryMax = 0; // MB
            uint32_t m_swapMax = 0; // MB
            unsigned m_cores = 0;

            // misc
            Timer* m_timer = nullptr;
            Timer* m_updateTimer = nullptr;
    };
}
#endif // SYSTEMSTATS_H
