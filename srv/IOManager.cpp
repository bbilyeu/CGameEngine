#include "srv/IOManager.h"
#include <fstream>
#include <sys/stat.h>

namespace CGameEngine
{
    bool IOManager::doesFileExist(std::string filePath)
    {
        struct stat buffer;
        return (stat (filePath.c_str(), &buffer) == 0);
        /*std::ifstream file;
        file.open(filePath);
        if(file.is_open())
        {
            file.close();
            return true;
        }

        // else
        return false;*/
    }

    bool IOManager::doesFolderExist(std::string folderPath)
    {
        return doesFileExist(folderPath);
    }

    bool IOManager::readFileToBuffer(std::string filePath, std::vector<unsigned char>& buffer)
    {
        std::ifstream file(filePath, std::ios::binary);
        if(file.fail())
        {
            perror(filePath.c_str());
            return false;
        }

        // seek to the end of the file, placing logical "cursor" there
        file.seekg(0, std::ios::end);

        // get file size
        int fileSize = file.tellg();

        // move "cursor" to beginning again
        file.seekg(0, std::ios::beg);
        fileSize -= file.tellg(); // disregard header. Likely irrelevant, but whatever

        buffer.resize(fileSize);
        file.read((char *)&(buffer[0]), fileSize);   // read number of bytes into the buffer, starting at the beginning of buffer

        file.close();

        return true;
    }

    bool IOManager::readFileToBuffer(std::string filePath, std::string& buffer)
    {
        std::ifstream file(filePath, std::ios::binary);
        if(file.fail())
        {
            perror(filePath.c_str());
            return false;
        }

        // seek to the end of the file, placing logical "cursor" there
        file.seekg(0, std::ios::end);

        // get file size
        int fileSize = file.tellg();

        // move "cursor" to beginning again
        file.seekg(0, std::ios::beg);
        fileSize -= file.tellg(); // disregard header. Likely irrelevant, but whatever

        buffer.resize(fileSize);
        file.read((char *)&(buffer[0]), fileSize);   // read number of bytes into the buffer, starting at the beginning of buffer

        file.close();

        return true;
    }
}
