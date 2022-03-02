#include "srv/Logger.h"
#include "srv/Time.h"
#include <iostream>

const std::chrono::milliseconds interval = std::chrono::milliseconds(250);

Logger::Logger()
{
    // tick on logging
    m_logging = true;
    m_active = true;
}

Logger::~Logger()
{
    if(m_fs)
    {
        std::string endLine = "----Closing Log----\n\n";
        m_fs->write(endLine.c_str(), endLine.length());
        m_fs->flush();
        m_fs->close();
        safeDelete(m_fs);
    }
    m_logging = false;
    m_active = false;
    m_messages.clear();
    /*if(m_updateThread)
    {
        while(!m_updateThread->joinable()) { std::cout<<"Logger::~Logger : Update Thread is not joinable...\n"; } // waiting...

        m_updateThread->join();
        if(m_updateThread) { delete m_updateThread; m_updateThread = nullptr; }
    }*/
}

Logger& Logger::getInstance()
{
	static Logger instance;
	return instance;
}

void Logger::setFilePath(std::string fp)
{
    m_toFile = true;
    m_filePath = fp;
    if(m_fs) { m_fs->close(); safeDelete(m_fs); }

    m_fs = new std::ofstream(fp, std::ofstream::out | std::ofstream::trunc);
    if(m_fs->bad() || m_fs->fail()) { std::cout<<txtbold + txtred + "ERROR" + txtnocolor + ": Failed to open file ("<<fp<<")!\n"; }
    else
    {
        std::string logStart = "\n----Staring Log----\n";
        m_fs->write(logStart.c_str(), logStart.length());
        logStart = "Start time is '" + CGameEngine::Time::getInstance().getTimestamp() + "'\n";
        m_fs->write(logStart.c_str(), logStart.length());
    }
}

/// Logger private functions //////////////////////////////////////////////////

void Logger::log(uint8_t lvl, uint8_t serv, const std::string funct, const std::string msg)
{
    if(lvl > m_logLevel || !m_logging || lvl >= Logs::NOLOG) { return; } // quick exit

    // built tail first from function name and message
    std::string message = funct + " :: " + msg;
    std::string ts = CGameEngine::Time::getInstance().getTimestamp(true);

    // set initial color
    switch (lvl)
    {
        case Logs::INFO:
        {
            message = ts + " [" + txtbold + "INFO" + txtnocolor + "] " + message + txtnocolor + "\n";
            break;
        }
        case Logs::WARN:
        {
            message = ts + " [" + txtyellow + "WARN" + txtnocolor + "] " + message + txtnocolor + "\n";
            break;
        }
        case Logs::CRIT:
        {
            message = ts + " [" + txtbold + txtred + "CRIT" + txtnocolor + "] " + message + txtnocolor + "\n";
            break;
        }
        case Logs::VERBOSE:
        {
            message = ts + " [" + txtcyan + "VERBOSE" + txtnocolor + "] " + message + txtnocolor + "\n";
            break;
        }
        case Logs::DEBUG:
        {
            message = ts + " [" + txtpurple + "DEBUG" + txtnocolor + "] " + message + txtnocolor + "\n";
            break;
        }
        case Logs::FATAL:
        {
            message = ts + " [" + txtbold + txtred + "FATAL" + "] " + message + txtnocolor + "\n";
            break;
        }
        default:
        {
            // unknown type!
            return;
        }
    }

    if(m_directDump) { std::cout<<message; } // immediately cout
    else if(m_toFile && m_fs) { m_fs->write(message.c_str(), message.length()); m_fs->flush(); }
    else { } // do nothing

    if(lvl == Logs::FATAL) { exit(1); }
}

/*void Logger::updateLoop()
{
    std::cout<<"Logger LogLevel ["<<(int)m_logLevel<<"]\n";
    while(m_active)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(m_mcv.wait_for(lock, interval, [&]{ return (m_messages.size() > 0); } ))
        {
            while(m_messages.size() > 0)
            {
                if(m_toConsole) { std::cout<<m_messages.front().second<<"\n"; }

                if(m_toFile && m_filePath != "")
                {
                    /// \TODO: implement logic to write to file
                }

                // remove from queue
                m_messages.pop();
            }

            // release lock
            lock.unlock();
        }
    }
}*/
