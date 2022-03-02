#ifndef DATABASE_H
#define DATABASE_H

#include "common/types.h"
#include "common/Timer.h"
#include "common/SafeQueue.h"
#include <mysqlx/xdevapi.h>
#include <mysqlx/xapi.h>
#include <mysqlx/devapi/result.h>
#include "common/QueryResult.h"
#include <string>

#define safeDeleteSession(sess) if(sess) { sess->close(); delete sess; sess=nullptr; }
//#define safeDeleteResultSet(rs) if(rs && rs.hasData()) { mysql_free_result(rs); }
#define safeDeleteStatement(stmt) if(stmt) { stmt->close(); delete stmt; stmt=nullptr; }

/*
    execute / executeQuery / executeUpdate : http://stackoverflow.com/questions/27480741/which-execute-function-should-i-use-in-mysql-connector-c
    Version 8.0 ::	https://dev.mysql.com/doc/dev/connector-cpp/8.0/usage.html
					https://dev.mysql.com/doc/dev/connector-cpp/8.0/devapi_ref.html

					https://dev.mysql.com/doc/x-devapi-userguide/en/images/plantuml-3fb9f98a36e345d538e7df2235eac467f1246121.png
					https://dev.mysql.com/doc/x-devapi-userguide/en/crud-operations-overview.html <-- super useful visual

*/

/// \TODO: Look in to prepared statements
/// \TODO: Avoid storing password in memory, look into SSL mysql connection(s)

namespace CGameEngine
{
    class Database
    {
        public:
            Database() { }
            Database(std::string host, std::string user, std::string pass, std::string schema, int port = 3306);
            virtual ~Database();
            void close();
            bool connect();
            bool isUp();
            //void asyncQuery(std::string queryStr, MYSQL_RES* resultSet);
            bool query(std::string queryStr);
            bool query(std::string queryStr, mysqlx::SqlResult& sqlRes);
            bool queryBool(std::string queryStr, bool& val);
            bool queryDouble(std::string queryStr, double& val);
            bool queryFloat(std::string queryStr, float& val);
            bool queryInt(std::string queryStr, int& val);
            bool queryString(std::string queryStr, std::string& val);



        private:
            mysqlx::Session* m_session = nullptr;

            // connection specifics
            std::string m_host = "";
            std::string m_user = "";
            std::string m_pass = "";
            std::string m_schema = "";
            int m_port = 3306;

            //void dbInit();

            SafeQueue<std::string> m_selectQueries;
            SafeQueue<std::string> m_otherQueries;
            Timer updateTimer = Timer("updateTimer", 3, TimeUnits::Seconds);

            bool querySingleValue(std::string queryStr, mysqlx::Value& sqlVal);
    };
}

#endif // DATABASE_H
