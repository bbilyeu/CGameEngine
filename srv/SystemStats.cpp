#include "SystemStats.h"
#include <thread>

namespace CGameEngine
{
    void SystemStats::update()
    {
        // prevent excessive updates
        if(!m_updateTimer->isExpired()) { return; }

        /// \SEE: https://stackoverflow.com/a/64166/6845246
        #if PLATFORM == PLATFORM_WINDOWS
            MEMORYSTATUSEX status;
            status.dwLength = sizeof(status);
            GlobalMemoryStatusEx( &status );

            // memory
            m_memoryArray[m_MA] = static_cast<float>(dwMemoryLoad);

            // cpu
            m_loadAverage = 0.0f; /// \TODO: HOW CAN THIS BE FOUND?!

            // swap
            m_swapUsage = static_cast<float>((status.ullTotalPageFile - status.ullAvailPageFile) / status.ullTotalPageFile);

            // uptime
            m_uptime = 0;
        #else
            struct sysinfo info;
            sysinfo( &info );

            // 15min CPU average (1, 5, 15)
            m_loadAverage = static_cast<float>(info.loads[2]/(float)LINUX_SYSINFO_LOADS_SCALE);

            // calculate memory average
            float total = info.totalram/(float)LINUX_SYSINFO_LOADS_SCALE;
            float freeR = info.freeram/(float)LINUX_SYSINFO_LOADS_SCALE;
            m_memoryArray[m_MA] = ((freeR / total) - 1.0f) * -100.0f;

            // swap
            total = info.totalswap/(float)LINUX_SYSINFO_LOADS_SCALE;
            freeR = info.freeswap/(float)LINUX_SYSINFO_LOADS_SCALE;
            if( (freeR/total) > 0.99f ) { m_swapUsage = 0.0f; }
            else { m_swapUsage = ((freeR / total) - 1.0f) * -100.0f; }

            // uptime
            m_uptime = static_cast<uint32_t>(info.uptime);
        #endif

        // memory average
        if(m_memoryArray[19] == 0.0f) { m_memoryAverage = m_memoryArray[m_MA]; } // assume previous value until filled
        else
        {
            float avg = 0.0f;
            for(int i = 0; i < 20; i++) { avg+=m_memoryArray[i]; }
            m_memoryAverage = static_cast<float>(avg / 20.0f);

        }
        m_MA = (m_MA == 19) ? 0 : (m_MA+1); // Reset or increment

        // CPU hourly average
        if(m_timer->isExpired())
        {
            m_loadArray[m_LA] = m_loadAverage; // store load average
            m_LA = (m_LA == 3) ? 0 : (m_LA+1); // Reset or increment
            m_timer->restart(); // restart timer

            // calculate hourly average, based on EXISTING averages so far
            float avg = 0.0f; int last = 0;
            for(int i = 0; i < 4; i++)
            {
                last++;
                if(m_loadArray[i] > 0.0f) { avg+=m_loadArray[i];}
                else { i = 99; } // end
            }
            m_loadAverageHourly = static_cast<float>(avg / (float)last);
        }

        // reset timer
        m_updateTimer->restart();
    }

    /// SystemStats private functions /////////////////////////////////////////

    SystemStats::SystemStats()
    {
        #if PLATFORM == PLATFORM_WINDOWS
            MEMORYSTATUSEX status;
            status.dwLength = sizeof(status);
            GlobalMemoryStatusEx(&status);
            m_memoryMax = static_cast<uint32>(status.ullTotalPhys / 1024 / 1024);
            m_swapMax = static_cast<uint32>(status.ullTotalPageFile / 1024 / 1024);
        #else // Linux
            struct sysinfo info;
            sysinfo( &info );
            m_memoryMax = static_cast<uint32_t>((info.totalram * info.mem_unit) / 1024 / 1024); // bytes -> kilobytes -> megabytes
            m_swapMax = static_cast<uint32_t>((info.totalswap * info.mem_unit) / 1024 / 1024); // bytes -> kilobytes -> megabytes
        #endif

        m_updateTimer = new Timer("updateTimer", 1, TimeUnits::Seconds);
        m_timer = new Timer("loadArray", 15, TimeUnits::Minutes);
        m_cores = std::thread::hardware_concurrency();
    }

    SystemStats::~SystemStats()
    {
        //dtor
    }
}
