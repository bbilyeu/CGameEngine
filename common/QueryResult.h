#ifndef QUERYRESULT_H_INCLUDED
#define QUERYRESULT_H_INCLUDED
#include <mysqlx/devapi/result.h>
#include <unordered_map>

class QueryResult : public mysqlx::SqlResult
{
	public:
		friend void copy(QueryResult& dst, const QueryResult& src) { if(&dst != &src) { dst.m_row = src.m_row; } }
        friend void swap(QueryResult& dst, QueryResult& src) { if(&dst != &src) { std::swap(dst.m_row, src.m_row); } }

		QueryResult() : mysqlx::SqlResult() {};
		QueryResult(const QueryResult& sr) { copy(*this, sr); } // copy ctor
		QueryResult(QueryResult&& sr) noexcept { swap(*this, sr); } // move ctor
		QueryResult& operator=(const QueryResult& sr) { copy(*this, sr); return *this; } // copy assignment
		QueryResult& operator=(QueryResult&& sr) noexcept { swap(*this, sr); return *this; } // move assignment
		/*QueryResult(const mysqlx::SqlResult r) : mysqlx::SqlResult() { mapBuild(); }; // copy ctor
		QueryResult(mysqlx::SqlResult&& r) noexcept : mysqlx::SqlResult() { mapBuild(); } // move ctor
		QueryResult& operator=(const mysqlx::SqlResult& r) { copy(*this, r); mapBuild(); return *this; } // copy assignment
		QueryResult& operator=(mysqlx::SqlResult&& r) noexcept { swap(*this, r); mapBuild(); return *this; } // move assignment*/

		~QueryResult()
		{
			if(m_row) { delete m_row; m_row = nullptr; }
			mysqlx_free(this);
		}

		bool isNull()
		{
            return (this->count() == 0);
		}

		bool next()
		{
			return mapBuild();
		}

		int getInt(std::string colName)
		{
			mysqlx::Value& val = getValue(colName);
			if(val.getType() == mysqlx::Value::Type::INT64) { return (int)val; }
			else { return 0; }
		}

		unsigned int getUInt(std::string colName)
		{
			mysqlx::Value& val = getValue(colName);
			if(mysqlx::Value::Type::UINT64) { return (unsigned int)val; }
			else { return 0; }
		}

		float getFloat(std::string colName)
		{
			mysqlx::Value& val = getValue(colName);
			if(val.getType() == mysqlx::Value::Type::FLOAT) { return (float)val; }
			else { return 0.0f; }
		}

		double getDouble(std::string colName)
		{
			mysqlx::Value& val = getValue(colName);
			if(val.getType() == mysqlx::Value::Type::DOUBLE) { return (double)val; }
			else { return 0; }
		}

		std::string getString(std::string colName)
		{
			mysqlx::Value& val = getValue(colName);
			if(val.getType() == mysqlx::Value::Type::STRING) { return (std::string)val; }
			else { return std::string("NULL"); }
		}


	private:
		std::unordered_map<std::string, mysqlx::Value>* m_row = nullptr;

		bool mapBuild()
		{
			if(m_row) { delete m_row; m_row = nullptr; }

			if(this->count() > 0)
			{
				m_row = new std::unordered_map<std::string, mysqlx::Value>();
				const mysqlx::Columns& columns = getColumns();
				unsigned int colCount = getColumnCount();
				mysqlx::Row row = fetchOne();
				if(!row.isNull() && colCount > 0)
				{
					for(unsigned int i = 0; i < colCount; i++)
					{
						m_row->insert( {columns[i].getColumnName(), row[i]} );
					}
					// success
					return true;
				}
			}
			// failure
			return false;
		}

		mysqlx::Value& getValue(std::string colName)
		{
			// ensure we actually have data
			if(!m_row)
			{
				if(!mapBuild())
				{
					throw std::runtime_error("QueryResult::getValue() has no data!");
				}
			}

			// try to access the data "safely"
			try
			{
				mysqlx::Value& val = m_row->at(colName);
				if(!val.isNull()) {	return m_row->at(colName); }
				else { throw std::runtime_error("QueryResult::getValue() is null!"); }
			}
			catch(const std::exception& e)
			{
				throw std::runtime_error("QueryResult::getValue() -- Invalid column!");
			}
		}

};

#endif // QUERYRESULT_H_INCLUDED
