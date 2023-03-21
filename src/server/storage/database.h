#pragma once
#include "../common/server_defs.h"
#include "table.h"
#include <stdio.h>
#include <map>
#include <unordered_map>
class DataBase
{
public:
    DataBase(const char *database_name);
    RE initialize();
    void destroy();
    const char *getDBName() { return database_name_.c_str(); }
    const char *getDBPath() { return database_path_.c_str(); }
    RE createTable();
private:
    RE create();
    RE destruction();
    bool isExists();
    std::string database_name_;
    std::string database_path_;
    std::unordered_map<std::string,Table*> opended_tables;
};


class DataBaseManager
{
public:
    static DataBaseManager &getInstance();
    void initialize();
    RE createDB(const char *database_name);
    DataBase* findDB(const char *databse_name);
    RE delDB(const char *database_name);
    RE closeDB(const char *database_name);
    void destroy();
private:
    DataBaseManager();
    ~DataBaseManager();
    std::map<const char *, DataBase *> opened_databases;
};

