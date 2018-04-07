#pragma once
#include <string>
#include <sqlite3.h>
#include <stdexcept>

class Database
{
public:
    sqlite3* m_sqlite;
    Database(std::string databasepath);
    ~Database();
};

#include "Database.tpp"
