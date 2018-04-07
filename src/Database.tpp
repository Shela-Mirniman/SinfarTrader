#include "Database.h"

Database::Database(std::string databasepath)
{
    if(sqlite3_open(databasepath.c_str(),&m_sqlite)!= SQLITE_OK)
    {
        throw std::runtime_error(sqlite3_errmsg(m_sqlite));
    }
}

Database::~Database()
{
    if(sqlite3_close(m_sqlite)!= SQLITE_OK)
    {
        std::cerr<<"In Database Destructor "<<sqlite3_errmsg(m_sqlite)<<std::endl;
    }
}
