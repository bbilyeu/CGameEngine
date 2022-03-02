#ifndef MEMORYMAPPEDFILE_H
#define MEMORYMAPPEDFILE_H

#include "common/types.h"
#include <mutex>
#include <condition_variable>
#include <string>

/*
    Ref:
        https://github.com/yhirose/cpp-mmaplib/blob/master/mmaplib.h
        http://man7.org/linux/man-pages/man2/mmap.2.html
        http://pubs.opengroup.org/onlinepubs/009695399/basedefs/sys/stat.h.html
		https://github.com/yhirose/cpp-mmaplib/blob/master/mmaplib.h
		http://www.goldsborough.me/c/c++/linker/2016/03/30/19-34-25-internal_and_external_linkage_in_c++/
*/

namespace CGameEngine
{
    /// \TODO: Should those be initialized on declaration?
    ///     Or should it be a nullptr that is created in zeroFill()?
    struct ShareStruct
    {
        uint32_t totalSize;
        unsigned char data[1];
    };

    class MemoryMutex
    {
        public:
            MemoryMutex(std::string fileName);
            ~MemoryMutex();
            bool lock();
            bool unlock();
            const bool& isLocked() const { return m_locked; }
            const int& getFD() const { return m_fd; }
            const std::string& getName() const { return m_name; }

        private:
            bool m_locked = false;
            bool m_valid = false;
            int m_fd = -1;
            std::string m_name = "";
            std::mutex m_mutex;
            std::condition_variable m_cv;
    };

    class MemoryMappedFile
    {
        public:
            MemoryMappedFile(std::string& writeFilePath, uint32_t fileSize); // create initial file (new)
            MemoryMappedFile(std::string& readFilePath); // read mapped memory file
            virtual ~MemoryMappedFile();
            void zeroFill();
            bool sync();

            void* operator->() const { return m_shared->data; }
            void* getData() const { return m_shared->data; }
            const int& getFD() const { return m_fd; }
            const uint32_t& getSize() const { return m_shared->totalSize; }

        private:
            MemoryMappedFile(const MemoryMappedFile&); // copy ctor
            const MemoryMappedFile& operator=(const MemoryMappedFile&); // assignment operator

            ShareStruct* m_shared = nullptr;
            int m_fd = -1;
            std::string m_path = "";
            bool m_readOnly = true;
            size_t m_size = 0;
    };
}

#endif // MEMORYMAPPEDFILE_H
