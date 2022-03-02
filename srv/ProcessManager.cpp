#include "srv/ProcessManager.h"

//#include <sys/types.h>

namespace CGameEngine
{
    PID ProcessManager::start(Process** proc)
    {
        // quick exit on nullptr
        if(*proc == nullptr) { return -1; }

        // taking ownership
        Process* p = *proc;
        proc = nullptr;

        ///  Example: char *argv[] = {"/bin/ps", "-u", "laser", 0};
        // recreate argv group
        int argVecSize = p->argVec.size();
        int argvc = argVecSize + 2;
        auto argv = new char*[argvc];
        argv[0] = const_cast<char*>(p->programPath.c_str());
        for(unsigned int i = 1; i <= argVecSize; i++)
        {
            argv[i] = const_cast<char*>(p->argVec[i-1].c_str());
        }
        argv[argvc-1] = nullptr;

        // fork process off
        PID pid = fork();

        if(pid == -1)
        {
            safeDeleteArray(argv);
            safeDelete(proc);
            return PIDerr;
        }
        else if(pid == 0) // child process
        {
            execv(argv[0], argv); // will launch process on success
            //_exit(1); // if this is hit, previous step failed HARD (maybe?)
        }

        // track newly created process
        m_processList.insert(pid, p);

        // cleanup
        safeDeleteArray(argv);

        return pid;
    }

    bool ProcessManager::pKill(const PID& pid, bool sig9 /*= false*/)
    {
        if(pid <= 0) { return false; } // quick exit to avoid entire server being killed

        SafeUnorderedMap<PID, Process*>::iterator it = m_processList.find(pid);
        if(it != m_processList.end()) // find process
        {
            int sig = (sig9 == true) ? SIGKILL : SIGTERM; // set signal to send
            if(kill(pid, sig) == -1) { return false; } // if failed on "kill -9 <pid>"
            else { m_processList.erase(it); return true; } // kill finally succeeded
        }
        else { return false; } // problems yo
    }

    void ProcessManager::pKillAll(bool sig9 /*= true*/, bool flush /*= false*/)
    {
        // 'flush' used as a "shutdown" flag

        SafeUnorderedMap<PID, Process*>::iterator pit;
        for(pit = m_processList.begin(); pit != m_processList.end(); ++pit)
        {
            pKill(pit->first, sig9);
            if(flush) { safeDelete(pit->second); }
        }

        if(flush) { m_processList.clear(); }
    }

    void ProcessManager::update()
    {
        while(m_signalsCaught > 0)
        {
            // decrease counter
            m_signalsCaught--;
            if(m_signalsCaught < 0) { m_signalsCaught = 0; } // *should* never happen

            int retStat = 0;

            // waitpid(): on success, returns the process ID of the child whose state has changed;
            // if WNOHANG was specified and one or more child(ren) specified by pid exist,
            // but have not yet changed state, then 0 is returned. On error, -1 is returned.
            PID pid = waitpid(-1, &retStat, WNOHANG);

            switch(pid)
            {
                case -1: // should never occur
                   Logger::getInstance().Log(Logs::CRIT, Logs::Processes, "ProcessManager::update()", "Err -1 returned from waitpid. retStat [{}]", retStat);
                    break;
                case 0: // nothing broken/found
                    break;
                default: // pid returned that has died/killed/etc
                {
                   Logger::getInstance().Log(Logs::VERBOSE, Logs::Processes, "ProcessManager::update()", "PID [{}] returned as dead/stopped.", pid);
                    SafeUnorderedMap<PID, Process*>::iterator it;
                    it = m_processList.find(pid);
                    if(it != m_processList.end())
                    {
                       Logger::getInstance().Log(Logs::VERBOSE, Logs::Processes, "ProcessManager::update()", "PID [{}] gone, running cleanup()", pid);
                        cleanupProcess(it);
                    }
                    else
                    {
                       Logger::getInstance().Log(Logs::WARN, Logs::Processes, "ProcessManager::update()", "PID [{}] not found within m_processList! retStat [{}]", pid, retStat);
                    }
                    break;
                }
            }
        }
    }


/// Process functions /////////////////////////////////////////////////////////

    void copy(Process& dst, const Process& src)
    {
        dst.programPath = src.programPath;
        dst.port = src.port;
        dst.handle = src.handle;
    }

    void swap(Process& dst, Process& src)
    {
        std::swap(dst.programPath, src.programPath);
        std::swap(dst.port, src.port);
        std::swap(dst.handle, src.handle);
    }

/// ProcessManager protected functions ////////////////////////////////////////

    ProcessManager::ProcessManager()
    {
        if(signal(SIGCHLD, ProcessManager::processChildSignal) == SIG_ERR)
        {
           Logger::getInstance().Log(Logs::FATAL, Logs::Processes, "ProcessManager::ProcessManager()", "Child signal handler could not be registered!");
		}
        m_signalsCaught = 0;
    }

    void ProcessManager::cleanupProcess(SafeUnorderedMap<PID, Process*>::iterator &it)
    {
        // if ProcessHandle exists, call it's cleanup()
        if(it->second->handle) { it->second->handle->cleanup(it->first, it->second); }

        // remove it
        safeDelete(it->second);
        m_processList.erase(it);
    }

/// ProcessManager private functions //////////////////////////////////////////

    void ProcessManager::processChildSignal(int sig)
    {
        if(sig == SIGCHLD)
        {
           Logger::getInstance().Log(Logs::WARN, Logs::Processes, "ProcessManager::processChildSignal()", "Caught SIGCHLD");
            ProcessManager::getInstance().m_signalsCaught++;
        }
    }
}
