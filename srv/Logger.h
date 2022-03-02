#ifndef LOGGER_H
#define LOGGER_H

#include "common/SafeQueue.h"
#include <string>
#include <queue>
#include <fstream>

#define FMT_HEADER_ONLY
#include "fmt/format.h"

/*
    Ref:
        http://fmtlib.net/latest/syntax.html
            Format syntax
        https://www.devarticles.com/c/a/Cplusplus/C-plus-plus-In-Theory-The-Singleton-Pattern-Part-I/
            Singletons with logging! GOLD!

*/

/// \TODO: Add output to file

namespace Logs
{
    enum LEVEL
    {
        FATAL = 0,  // dun goofed
        CRIT,       // very bad, but not crash-worthy
        WARN,       // likely coding error
        INFO,       // minimal one-liners
        VERBOSE,    // "there is no kind of kill, like overkill"
        DEBUG,      // debugging output only
        NOLOG       // dead end value
    };

    enum CATEGORY
    {
        None = 0, // unknown
        Audio,
        Camera,			// game cameras
        Core,			// CGameEngine core, *-launcher.cpp, main.cpp, etc
        Database,		// queries, db connections, valid db pointer
        DataValidation,	// anywhere where a created object may have failed or be null/nullptr
        Drawing,		// any form of rendering, debug rendering, or prepare-for-rendering
        Gameplay,		// should be client specific
        Generic, 		// for debug messages
        GLSLProgram,	// shader only issues
        GUI,			// GUI anything (excluding drawing)
        IO,				// file read/write error, unable to obtain file lock, etc
        Login,
        MemoryMapping,	// specific to MemoryMappedFile access/generation. Populating goes to Database and/or IO
        Network,        // Anything from packets, sockets, network services, or connections
        Physics,
        Processes,		// Process management and threading
        Service,		// Semi generic for things like zones, zone controllers, zone containers, etc
        Utility,		// glm_util.h, util.h, etc (header-only functions)
        END
    };
}

/*#ifndef LOGGER_LEVEL
#define LOG(...) do {} while (0)

#elif LOGGER_LEVEL
#define LOG(...) \
    do { if(lvl <= LogSys.getLogLevel())Log(...); } while (0)*/

class Logger
{
    public:
        // allow access without needing to manually declare/pass-by-ref
        static Logger& getInstance();

        // allow passing of parameters printf styled
        template <typename... Args>
        void Log(uint8_t lvl, uint8_t serv, const std::string funct, const char *msg, const Args&... p)
        {
            std::string out = fmt::format(msg, p...); // convert to stringstream
            log(lvl, serv, funct, out); // call normal log
        }

        template <typename... Args>
        void Log(uint8_t lvl, const std::string funct, const char *msg, const Args&... p)
        {
			if(lvl != Logs::DEBUG) { return; }
			std::string out = fmt::format(msg, p...); // convert to stringstream
            log(lvl, Logs::Generic, funct, out); // call normal log
        }

        const uint8_t& getLogLevel() const { return m_logLevel; }
        void setFilePath(std::string fp);
        void setLogLevel(uint8_t lvl) { m_logLevel = lvl; }
        void showConsoleOutput(bool val) { m_toConsole = val; }
        void setDirectDump(bool val) { m_directDump = val; m_active = (val) ? false : true; } // used to immediately cout during Log()

    private:
		Logger();
		~Logger();
        Logger(const Logger&) = delete;
        Logger operator&(const Logger&) = delete;
        // actual logging function
        void log(uint8_t lvl, uint8_t serv, const std::string funct, const std::string msg);

		uint8_t m_logLevel = Logs::CRIT;
        bool m_active = false;
        bool m_directDump = false;
        bool m_logging = false;
        bool m_toConsole = true;
        bool m_toFile = false;

        SafeQueue<std::pair<uint8_t,std::string>> m_messages;
        std::string m_filePath = "";
        std::ofstream* m_fs = nullptr;

        const std::string txtred = "\033[91m";
        const std::string txtyellow = "\033[93m";
        const std::string txtgreen = "\033[92m";
        const std::string txtpurple = "\033[95m";
        const std::string txtcyan = "\033[96m";
        const std::string txtbold = "\033[1m";
        const std::string txtnocolor = "\033[0m";
};

#endif // LOGGER_H
