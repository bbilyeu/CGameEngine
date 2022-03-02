#include "Database.h"
#include <ctime>
#include <stdlib.h>
#include "srv/Time.h"
#include <poll.h>
#include "common/util.h"

namespace CGameEngine
{
//	static int dbPoll(MYSQL* dbConn, int status)
//	{
//		struct pollfd pfd;
//		int timeout, res;
//
//		pfd.fd = mysql_get_socket(dbConn);
//		pfd.events = 	(status & MYSQL_WAIT_READ ? POLLIN : 0) |
//						(status & MYSQL_WAIT_WRITE ? POLLOUT : 0) |
//						(status & MYSQL_WAIT_EXCEPT ? POLLPRI : 0);
//
//		if (status & MYSQL_WAIT_TIMEOUT) { timeout = 1000*mysql_get_timeout_value(dbConn); }
//		else { timeout = -1; }
//
//		// actual polling
//		res = poll(&pfd, 1, timeout);
//		if (res == 0) { return MYSQL_WAIT_TIMEOUT; }
//		else if (res < 0) { return MYSQL_WAIT_TIMEOUT; }
//		else
//		{
//			int status = 0;
//			if (pfd.revents & POLLIN) status |= MYSQL_WAIT_READ;
//			if (pfd.revents & POLLOUT) status |= MYSQL_WAIT_WRITE;
//			if (pfd.revents & POLLPRI) status |= MYSQL_WAIT_EXCEPT;
//			return status;
//		}
//	}

	Database::Database(std::string host, std::string user, std::string pass, std::string schema, int port /*= 3306 */)
		: m_host(host), m_port(port), m_schema(schema), m_user(user), m_pass(pass)
	{
		connect();
	}

    Database::~Database()
    {
        close();
    }

    void Database::close()
    {
		safeDeleteSession(m_session);
    }

    bool Database::connect()
    {
		if(m_session) { return true; }
		else
		{
			try
			{
				m_session = new mysqlx::Session(mysqlx::SessionOption::HOST, m_host,
												mysqlx::SessionOption::PORT, m_port,
												mysqlx::SessionOption::USER, m_user,
												mysqlx::SessionOption::PWD, m_pass);
				return true;
			}
			catch(std::exception& e)
			{
				Logger::getInstance().Log(Logs::CRIT, Logs::Database, "Database(...)",
					"Failed to create a session to host [{}], on port [{}], with user [{}], against schema [{}].\n\t'{}'", m_host, m_port, m_user, m_schema, e.what());
			}
		}

		return false;
    }

    /// \TODO: REVIEW THIS FOR CRASHES!
//    bool Database::connect()
//    {
//       Logger::getInstance().Log(Logs::VERBOSE, Logs::Database, "Database::connect()", "connect() called");
//        if(m_schema != "" && m_user != "" && m_pass != "")
//        {
//			int retVal = 0, errVal = 0;
//			const char* localhost = "127.0.0.1";
//			std::string queryStr = "SHOW STATUS";
//			MYSQL_RES *resultSet;
//			MYSQL_ROW row;
//
//			// build connection
//			dbInit();
//			retVal = mysql_real_connect_start(&sqlRet, &m_dbCon, localhost, m_user.c_str(), m_pass.c_str(), m_schema.c_str(), 0, nullptr, 0);
//			while (retVal)
//			{
//				retVal = dbPoll(&m_dbCon, retVal);
//				retVal = mysql_real_connect_cont(&sqlRet, &m_dbCon, retVal);
//			}
//
//			// exit on failure
//			if(!sqlRet)	{ Logger::getInstance().Log(Logs::FATAL, Logs::Database, "Database::connect()", "Failed on mysql_real_connect()"); } // TODO: Handle as CRIT, return false
//
//			// run test query
//			retVal = mysql_real_query_start(&errVal, &m_dbCon, queryStr.c_str(), queryStr.size());
//			while(retVal)
//			{
//				retVal = dbPoll(&m_dbCon, retVal);
//				retVal = mysql_real_query_cont(&errVal, &m_dbCon, retVal);
//			}
//
//			// exit on failure
//			if(errVal) { Logger::getInstance().Log(Logs::FATAL, Logs::Database, "Database::connect()", "Failed on connection test query."); } // TODO: Handle as CRIT, return false
//
//			// parse results of the test query
//			resultSet = mysql_use_result(&m_dbCon);
//			if(!resultSet) { Logger::getInstance().Log(Logs::FATAL, Logs::Database, "Database::connect()", "Failed accessing connection test query results."); } // TODO: Handle as CRIT, return false
//
//			// loop indefinitely until all data is read
//			for (;;)
//			{
//				retVal = mysql_fetch_row_start(&row, resultSet);
//				while (retVal)
//				{
//					retVal = dbPoll(&m_dbCon, retVal);
//					retVal = mysql_fetch_row_cont(&row, resultSet, retVal);
//				}
//
//				// break on no data left
//				if(!row) { break; }
//			}
//
//			// if any errors have appeared, fail out
//			if(mysql_errno(&m_dbCon)) { Logger::getInstance().Log(Logs::FATAL, Logs::Database, "Database::connect()", "Failed retrieving test query rows."); } // TODO: Handle as CRIT, return false
//
//
//			// clean up
//			mysql_free_result(resultSet);
//
//                // check for successful connection
////                if(m_dbcon->isValid())
////                {
////                    m_dbcon->setSchema(m_schema);
////                    m_listening = true;
////                    m_thread = new std::thread(sqlListenWrapper, this);
////                    return true;
////                }
//
//			// DB connection established
//			return true;
//        }
//
//        // else
//        return false;
//    }

    bool Database::isUp()
    {
        if(m_session)
        {
			// test query to ensure active session
			try
			{
				m_session->sql("SELECT NOW()");
				return true;
			}
			catch (std::exception& e)
			{
				// delete session and attempt to re-establish
				Logger::getInstance().Log(Logs::WARN, Logs::Database, "Database::isUp()", "Test query failed. Attempting to re-establish session. [{}]", e.what());
				safeDeleteSession(m_session);
			}

			/// \TODO: Determine if this is a risk by not executing a test query too
			if(connect()) { Logger::getInstance().Log(Logs::INFO, Logs::Database, "Database::isUp()", "Session rebuilt."); return true; }
			else
			{
				Logger::getInstance().Log(Logs::CRIT, Logs::Database, "Database::isUp()", "Database is down/missing, connect() attempt failed.");
				exit(-10); // TODO: Handle as CRIT, add logic to gracefully shutdown
            }
        }

        return false;
    }

    bool Database::query(std::string queryStr)
    {
        if(m_session)
        {
            mysqlx::SqlResult val = m_session->sql(queryStr).execute();
			if(val.getAffectedItemsCount() == 0) { Logger::getInstance().Log(Logs::INFO, Logs::Database, "Database::query()", "Query updated no records. [{}]", queryStr); }
			else { return true; }
        }
        else
		{
			Logger::getInstance().Log(Logs::CRIT, Logs::Database, "Database::query()", "Query attempted without existing Session. [{}]", queryStr);
		}

		return false;
    }

    bool Database::query(std::string queryStr, mysqlx::SqlResult& val)
    {
        if(m_session)
        {
			val = m_session->sql(queryStr).execute();
			if(!val.hasData()) { Logger::getInstance().Log(Logs::INFO, Logs::Database, "Database::query()", "Query returned no results. [{}]", queryStr); }
			else { return true; }
        }
        else
		{
			Logger::getInstance().Log(Logs::CRIT, Logs::Database, "Database::query()", "Query attempted without existing Session. [{}]", queryStr);
		}

		return false;
    }

    bool Database::queryBool(std::string queryStr, bool& val)
    {
		bool retVal = false;
		mysqlx::Value sqlVal;

		if(querySingleValue(queryStr, sqlVal))
		{
			if(sqlVal.getType() == mysqlx::Value::Type::BOOL)
			{
				val = bool(sqlVal);
				retVal = true;
			}
			else if(sqlVal.getType() == mysqlx::Value::Type::STRING)
			{
				std::string tmp = std::string(sqlVal);
				if(toLowerOut(tmp) == "t" || toLowerOut(tmp) == "true" || tmp == "1") { val = true; retVal = true; }
				else if(toLowerOut(tmp) == "f" || toLowerOut(tmp) == "false" || tmp == "0") { val = false; retVal = true; }
				else { Logger::getInstance().Log(Logs::WARN, Logs::Database, "Database::queryBool()", "Incompatible string value returned [{}], expected 't', 'f', 'true', 'false' (case insensitive).", tmp); }
			}
			else if(sqlVal.getType() == mysqlx::Value::Type::UINT64 || sqlVal.getType() == mysqlx::Value::Type::INT64)
			{
				int tmp = int(sqlVal);
				if(tmp == 1) { val = true; retVal = true; }
				else if(tmp == 0) { val = false; retVal = true; }
				else { Logger::getInstance().Log(Logs::WARN, Logs::Database, "Database::queryBool()", "Incompatible integer value returned [{}], expected '0' or '1'.", tmp); }
			}
			else { Logger::getInstance().Log(Logs::WARN, Logs::Database, "Database::queryBool()", "Incompatible type returned, expected BOOL."); }
		}
		return retVal;
    }

	bool Database::queryDouble(std::string queryStr, double& val)
    {
		mysqlx::Value sqlVal;
		if(querySingleValue(queryStr, sqlVal))
		{
			if(sqlVal.getType() == mysqlx::Value::Type::DOUBLE) { val = double(sqlVal); return true; }
			else { Logger::getInstance().Log(Logs::WARN, Logs::Database, "Database::queryDouble()", "Incompatible type returned, expected DOUBLE."); }
		}
		return false;
    }

    bool Database::queryFloat(std::string queryStr, float& val)
    {
		mysqlx::Value sqlVal;
		if(querySingleValue(queryStr, sqlVal))
		{
			if(sqlVal.getType() == mysqlx::Value::Type::FLOAT) { val = float(sqlVal); return true; }
			else { Logger::getInstance().Log(Logs::WARN, Logs::Database, "Database::queryFloat()", "Incompatible type returned, expected FLOAT."); }
		}
		return false;
    }

    bool Database::queryInt(std::string queryStr, int& val)
    {
		mysqlx::Value sqlVal;
		if(querySingleValue(queryStr, sqlVal))
		{
			if(sqlVal.getType() == mysqlx::Value::Type::INT64 || sqlVal.getType() == mysqlx::Value::Type::UINT64) { val = int(sqlVal); return true; }
			else { Logger::getInstance().Log(Logs::WARN, Logs::Database, "Database::queryInt()", "Incompatible type returned, expected INT64 or UINT64."); }
		}
		return false;
    }

    bool Database::queryString(std::string queryStr, std::string& val)
    {
		mysqlx::Value sqlVal;
		if(querySingleValue(queryStr, sqlVal))
		{
			if(sqlVal.getType() == mysqlx::Value::Type::STRING) { val = std::string(sqlVal); return true; }
			else { Logger::getInstance().Log(Logs::WARN, Logs::Database, "Database::queryString()", "Incompatible type returned, expected STRING."); }
		}
		return false;
    }

//    void Database::asyncQuery(std::string queryStr, MYSQL_RES* resultSet)
//    {
//		int retVal = 0, errVal = 0;
//
//		retVal = mysql_real_query_start(&errVal, &m_dbCon, queryStr.c_str(), queryStr.size());
//		while(retVal)
//		{
//			retVal = dbPoll(&m_dbCon, retVal);
//			retVal = mysql_real_query_cont(&errVal, &m_dbCon, retVal);
//		}
//
//		if(errVal) { Logger::getInstance().Log(Logs::CRIT, Logs::Database, "Database::asyncQuery()", "Failed with error code [{}] on query: [{}]", errVal, queryStr); return; } // TODO: Handle as CRIT, return false
//
//		resultSet = mysql_store_result(&m_dbCon);
//		if(!resultSet) { Logger::getInstance().Log(Logs::CRIT, Logs::Database, "Database::asyncQuery()", "Failed accessing query results on query: [{}]", queryStr); return; } // TODO: Handle as CRIT, return false
//
//		if(mysql_errno(&m_dbCon)) { Logger::getInstance().Log(Logs::CRIT, Logs::Database, "Database::asyncQuery()", "Failed retrieving result rows for query: [{}]", queryStr); return; }
//    }

//    void Database::query(std::string query, sql::ResultSet** rs)
//    {
//        m_selectQueries.push(query);
//        m_selectResults.push(rs);
//       Logger::getInstance().Log(Logs::VERBOSE, Logs::Database, "Database::query()", "Adding query to queue. Queries/Results [{} / {}]", m_selectQueries.size(), m_selectResults.size());
//    }


    /// private functions below ///////////////////////////////////////////////

//    void Database::dbInit()
//    {
//		if(!m_init)
//		{
//			m_init = true;
//			mysql_init(&m_dbCon);
//			mysql_options(&m_dbCon, MYSQL_OPT_NONBLOCK, 0);
//		}
//    }

	bool Database::querySingleValue(std::string queryStr, mysqlx::Value& sqlVal)
	{
		mysqlx::SqlResult retRes;
		mysqlx::Value retVal;

        if(m_session)
        {
			retRes = m_session->sql(queryStr).execute();
			if(!retRes.hasData() || retRes.count() == 0 || retRes.getColumnCount() == 0) { Logger::getInstance().Log(Logs::INFO, Logs::Database, "Database::querySingleValue()", "Query returned no results. [{}]", queryStr); }
			else if(retRes.count() > 1 || retRes.getColumnCount() > 1) { Logger::getInstance().Log(Logs::WARN, Logs::Database, "Database::querySingleValue()", "Query returned too many results [{}]", queryStr); }

			sqlVal = retRes.fetchOne()[0];
			return true;
        }
        else
		{
			Logger::getInstance().Log(Logs::CRIT, Logs::Database, "Database::querySingleValue()", "Query attempted without existing Session. [{}]", queryStr);
		}


		return false;
	}
}
