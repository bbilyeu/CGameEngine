#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include "common/types.h"
#include "srv/IOManager.h"

#include "sqlite3/sqlite3.h"

/*
    Ref:
        https://www.sqlite.org/quickstart.html
        https://www.sqlite.org/cintro.html
        https://stackoverflow.com/questions/31146713/sqlite3-exec-callback-function-clarification

*/

/// \TODO: Look in to prepared statements

namespace CGameEngine
{
    class SQLiteDatabase
    {
        public:
            SQLiteDatabase(std::string filePath)
            {
				if(IOManager::doesFileExist(filePath)) // test file existence
				{
					m_filePath = filePath;
					if(tryOpenConnection()) { sqlite3_close(m_dbcon); }
					else { Logger::getInstance().Log(Logs::FATAL, Logs::Database, "SQLiteDatabase::SQLiteDatabase()", "Failed to open database at '{}'. {}", m_filePath, sqlite3_errmsg(m_dbcon)); }
				}
				else // file does not exist
				{
					Logger::getInstance().Log(Logs::FATAL, Logs::Database, "SQLiteDatabase::SQLiteDatabase()", "DB File '{}' does not exist!", filePath);
				}
			}
            virtual ~SQLiteDatabase() { sqlite3_close(m_dbcon); m_filePath = ""; }
            void setFilePath(std::string filePath) { sqlite3_close(m_dbcon); m_filePath = filePath; }

            void query(std::string statement, int (*callback)(void*,int,char**,char**), char* errorMessage)
            {
                if(m_filePath == "")
                {
                   Logger::getInstance().Log(Logs::CRIT, Logs::Database, "SQLiteDatabase::SQLiteDatabase()", "No SQLite db file specified!");
                    return;
                }

                if(tryOpenConnection()) // try to open db
                {
                    int status = sqlite3_exec(m_dbcon, statement.c_str(), callback, 0, &errorMessage);
                    if(status != SQLITE_OK)
                    {
                       Logger::getInstance().Log(Logs::CRIT, Logs::Database, "SQLiteDatabase::SQLiteDatabase()", "Query Failure '{}'. Message: {}", m_filePath, errorMessage);
                        sqlite3_free(errorMessage);
                        return;
                    }
                }
                else // somehow the connection failed, this should not happen
                {
                   Logger::getInstance().Log(Logs::CRIT, Logs::Database, "SQLiteDatabase::SQLiteDatabase()", "SQLite db file '{}' failed to open!", m_filePath);
                }
            }


        private:
            sqlite3* m_dbcon = nullptr;
            std::string m_filePath = "";

            bool tryOpenConnection()
            {
                if(m_dbcon) { sqlite3_close(m_dbcon); }                     // close existing connection
                int status = sqlite3_open(m_filePath.c_str(), &m_dbcon);    // attempt to open db file
                if(status)                                                  // if non-zero returned
                {
                   Logger::getInstance().Log(Logs::CRIT, Logs::Database, "SQLiteDatabase::SQLiteDatabase()", "Failed to open database at '{}'. {}", m_filePath, sqlite3_errmsg(m_dbcon));
                    sqlite3_close(m_dbcon);
                    return false;
                }
                // else, success
                return true;
            }

    };
}

#endif // SQLITEDATABASE_H
