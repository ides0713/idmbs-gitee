#pragma once

#include "../common/server_defs.h"
#include "table.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <unordered_map>

class DataBase {
public:
    explicit DataBase(const char *database_name);

    Re initialize();

    void destroy();

    const char *getDbName() { return database_name_.c_str(); }

    const char *getDbPath() { return database_path_.c_str(); }

    Re createTable();

private:
//    Re create();

    Re destruction();

//    bool isExists();

    std::string database_name_;
    std::string database_path_;
    std::unordered_map<std::string, Table *> opened_tables_;
};


class DataBaseManager {
public:
    static DataBaseManager &getInstance();

    void initialize();

    Re createDb(const char *database_name);

    DataBase *findDb(const char *databse_name);

    Re delDb(const char *database_name);

    Re closeDb(const char *database_name);

    void destroy();

private:
    DataBaseManager();

    std::map<const char *, DataBase *> opened_databases_;
};

