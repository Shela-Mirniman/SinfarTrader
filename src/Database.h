#pragma once
#include <string>
#include <sqlite3.h>
#include <stdexcept>
#include <iostream>

class Database
{
public:
    sqlite3* m_sqlite;
    Database(std::string databasepath);
    ~Database();
};
