#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include "common/types.h"
#include "common/SafeUnorderedMap.h"
#include <sys/wait.h> // waitpid(), SIGTERM, SIGKILL, etc
#include <unistd.h>
#include <unordered_map>
#include <vector>

/*
    Ref:
        execv : https://stackoverflow.com/questions/5687928/execv-quirks-concerning-arguments
        fork : https://stackoverflow.com/questions/35300580/does-the-c-standard-support-processes
        _exit : https://stackoverflow.com/questions/5422831/what-is-the-difference-between-using-exit-exit-in-a-conventional-linux-fo
        waitpid :
            https://stackoverflow.com/questions/21248840/example-of-waitpid-in-use
            https://linux.die.net/man/2/waitpid
*/

typedef pid_t PID;
static const PID PIDerr = -1; // used for last failed process ID

namespace CGameEngine
{
    // forward decl
    class Process;
    class ProcessHandle;

    class ProcessManager
    {
        public:
            static ProcessManager& getInstance() { static ProcessManager pm; return pm; }

            // start process
            PID start(Process** proc);

            // end process(es)
            bool pKill(const PID& pid, bool sig9 = false);
            void pKillAll(bool sig9 = true, bool flush = false);

            // handle process 'health' checks or simple reboots
            void update();

            // status 'gets'
            const bool empty() const { return m_processList.empty(); }
            const unsigned int size() const { return m_processList.size(); }

        protected:
            ProcessManager();
            virtual ~ProcessManager() {}; // should this kill off the processes?
            void cleanupProcess(SafeUnorderedMap<PID, Process*>::iterator& it);
            SafeUnorderedMap<PID, Process*> m_processList;

        private:
            static void processChildSignal(int sig);
            uint32_t m_signalsCaught = 0;
    };

    // info about actual program to launch
    class Process
    {
        public:
            Process() {} // for parsed declaration
            Process(std::string progPath) : programPath(progPath), argVec() {} // specify program
            Process(const Process& p) { copy(*this, p); } // copy ctor
            Process(Process&& p) noexcept { swap(*this, p); } // move ctor
            Process& operator=(const Process& p) { copy(*this, p); return *this; } // copy assignment
            Process& operator=(Process&& p) noexcept { swap(*this, p); return *this; } // move assignment

            std::string programPath = "";
            std::vector<std::string> argVec;
            uint16_t port = 0;
            ProcessHandle* handle = nullptr;
            /// \TODO: Add logging source (or should the logging be handled in the process called?)

            friend void copy(Process& dst, const Process& src);
            friend void swap(Process& dst, Process& src);
    };

    // pointer to handler class
    class ProcessHandle
    {
        public:
            virtual ~ProcessHandle() {} // clean up nothing
            virtual void cleanup(const PID& pid, const Process* process) = 0; // pure virtual hook to end of each handler
    };
}

#endif // PROCESSMANAGER_H
