#include "MemoryMappedFile.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdexcept>


namespace CGameEngine
{
    /// MemoryMutex ///////////////////////////////////////////////////////////

    MemoryMutex::MemoryMutex(std::string fileName)
    {
        m_name = fileName + ".lock";
        m_fd = open(m_name.c_str(), O_RDWR | O_CREAT | O_CLOEXEC, S_IRUSR | S_IWUSR);
        if(m_fd == -1) { Logger::getInstance().Log(Logs::FATAL, Logs::MemoryMapping, "MemoryMutex::MemoryMutex(string)", "Could not create a lock file with name '{}'!", m_name); }
        else { m_valid = true; }
    }

    MemoryMutex::~MemoryMutex()
    {
        if(m_fd > 0)
        {
            if(m_locked) { lockf(m_fd, F_ULOCK, 0); }
            close(m_fd);
        }
        if(m_name != "") { remove(m_name.c_str()); }
        m_fd = -1;
        m_locked = false;
        m_valid = false;
    }

    bool MemoryMutex::lock()
    {
        if(!m_valid) { Logger::getInstance().Log(Logs::FATAL, Logs::MemoryMapping, "MemoryMutex::lock()", "Attempted to lock with invalid lock, name '{}'!", m_name); }
        else if(m_locked) { return m_locked; } // already locked

        int retVal = 0;
        int counter = 5;
        while(counter > 0 && !m_locked)
        {
            std::unique_lock<std::mutex> llock(m_mutex);
            if(m_cv.wait_for(llock, std::chrono::milliseconds(100)) == std::cv_status::timeout)
            {
                counter--;
                retVal = lockf(m_fd, F_LOCK, 0);
                if(retVal == 0) { m_locked = true; }
            }
        }

        if(!m_locked) { Logger::getInstance().Log(Logs::CRIT, Logs::MemoryMapping, "MemoryMutex::lock()", "Attempted to lock with valid lock ({}, fd {}) but failed!", m_name, m_fd); }
        return m_locked;
    }

    bool MemoryMutex::unlock()
    {
        if(!m_valid) { Logger::getInstance().Log(Logs::FATAL, Logs::MemoryMapping, "MemoryMutex::unlock()", "Attempted to unlock with invalid lock, name '{}'!", m_name); }
        else if(!m_locked) { return m_locked; } // already unlocked

        int retVal = 0;
        int counter = 5;
        while(counter > 0 && m_locked)
        {
            std::unique_lock<std::mutex> llock(m_mutex);
            if(m_cv.wait_for(llock, std::chrono::milliseconds(100)) == std::cv_status::timeout)
            {
                counter--;
                retVal = lockf(m_fd, F_ULOCK, 0);
                if(retVal == 0) { m_locked = false; }
            }
        }

        if(m_locked) { Logger::getInstance().Log(Logs::CRIT, Logs::MemoryMapping, "MemoryMutex::unlock()", "Attempted to unlock with valid lock ({}, fd {}) but failed!", m_name, m_fd); }
        return m_locked;
    }

    /// MemoryMappedFile ///////////////////////////////////////////////////////////

    /// \WRITING data
    MemoryMappedFile::MemoryMappedFile(std::string& writeFilePath, uint32_t fileSize)
        : m_path(writeFilePath), m_readOnly(false)
    {
        // open fd
        m_fd = open(writeFilePath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if(m_fd == -1) { Logger::getInstance().Log(Logs::FATAL, Logs::MemoryMapping, "MemoryMappedFile::MemoryMappedFile(string, uint32_t)", "Failed to open fd for path '{}'!", writeFilePath); }

        // size file to suit our needs
        m_size = fileSize + sizeof(ShareStruct);
        if(ftruncate(m_fd, m_size) < 0) { Logger::getInstance().Log(Logs::FATAL, Logs::MemoryMapping, "MemoryMappedFile::MemoryMappedFile(string, uint32_t)", "Failed to resize file for path '{}', size of '{}'!", writeFilePath, fileSize); }

        // finally, create the shared file
        m_shared = reinterpret_cast<ShareStruct*>(mmap(nullptr, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0));
        if(m_shared == MAP_FAILED) { Logger::getInstance().Log(Logs::FATAL, Logs::MemoryMapping, "MemoryMappedFile::MemoryMappedFile(string, uint32_t)", "Failed to map file for fd '{}' and path '{}'!", m_fd, writeFilePath); }
    }

    /// \READING data
    MemoryMappedFile::MemoryMappedFile(std::string& readFilePath)
        : m_path(readFilePath)
    {
        // get file descriptor from filepath
		m_fd = open(readFilePath.c_str(), O_RDONLY, S_IRUSR);
		if(m_fd == -1) { Logger::getInstance().Log(Logs::FATAL, Logs::MemoryMapping, "MemoryMappedFile::MemoryMappedFile(string)", "Failed to open fd for path '{}'!", readFilePath); }

		// stat() file for size
		struct stat sb;
		if(fstat(m_fd, &sb) < 0) { Logger::getInstance().Log(Logs::FATAL, Logs::MemoryMapping, "MemoryMappedFile::MemoryMappedFile(string)", "Failed to stat file for fd '{}' and path '{}'!", m_fd, readFilePath);	}
		m_size = sb.st_size;

		// map to pointer
		m_shared = reinterpret_cast<ShareStruct*>(mmap(nullptr, m_size, PROT_READ, MAP_SHARED, m_fd, 0));
        if(m_shared == MAP_FAILED) { Logger::getInstance().Log(Logs::FATAL, Logs::MemoryMapping, "MemoryMappedFile::MemoryMappedFile(string)", "Failed to map file for fd '{}' and path '{}'!", m_fd, readFilePath); }
    }

    MemoryMappedFile::~MemoryMappedFile()
    {
    	// only allow 'writers' to delete
    	if(!m_readOnly)
    	{
			if(m_shared) { munmap(reinterpret_cast<void*>(m_shared), m_size); } // unmapping the memory from the file
        	if(m_path != "") { remove(m_path.c_str()); } // deleting the actual file
        }

		// close out file descriptor, if exists
        if(m_fd > 0)
        {
            close(m_fd);
            m_fd = -1;
        }

        m_size = 0;
        m_path = "";
    }

    void MemoryMappedFile::zeroFill()
    {
		if(m_readOnly) { return; } // can't let 'readers' make changes
        memset(reinterpret_cast<void*>(m_shared), 0, sizeof(ShareStruct));
        memset(m_shared->data, 0, m_shared->totalSize);
    }

    bool MemoryMappedFile::sync()
    {
		return (msync(m_shared->data, static_cast<size_t>(m_shared->totalSize), MS_SYNC));
    }
}
