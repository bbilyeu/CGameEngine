#ifndef IOMANAGER_H
#define IOMANAGER_H
#include <vector>
#include <string>

namespace CGameEngine
{
    /// \TODO: Does "stat" work on Windows?
    class IOManager
    {
        public:
            static bool doesFileExist(std::string filePath);
            static bool doesFolderExist(std::string folderPath);
            static bool readFileToBuffer(std::string filePath, std::vector<unsigned char>& buffer);
            static bool readFileToBuffer(std::string filePath, std::string& buffer);
    };
}

#endif // IOMANAGER_H
