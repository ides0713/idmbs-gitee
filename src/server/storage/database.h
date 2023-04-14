#pragma once

#include "../common/server_defs.h"
#include "table.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <map>
#include <unordered_map>
#include <filesystem>
#include "../parse/parse_defs.h"

class CLogManager;

#define DATABASE_NAME_MAX_LEN 20

class DataBase {
public:
    DataBase() = default;

    Re init(const char *database_name, const std::filesystem::path &database_path);

    void destroy();

    const std::string getDbName() { return database_name_; }

    std::filesystem::path const getDbPath() { return database_path_; }

    Re createTable(const char *table_name, const size_t attr_infos_num, const AttrInfo *attr_infos);

    Re createTable(const std::string &table_name, const size_t attr_infos_num, const AttrInfo *attr_infos);

    ///@brief existed table will be opened when database constructed,when table is creating,
    ///it will be added into opened_tables automatically,so all exist valid tables can be find in opened_tables
    Table *getTable(const std::string &table_name);

    CLogManager *getCLogManager() { return clog_manager_; }

private:
//    Re create();
    Re openAllTables();

    Re destruction();

//    bool isExists();
    CLogManager *clog_manager_;
    std::string database_name_;
    std::filesystem::path database_path_;
    std::unordered_map<std::string, Table *> opened_tables_;
};


class GlobalDataBaseManager {
public:
    GlobalDataBaseManager() = default;

    void init();

    Re createDb(const char *database_name);

    DataBase *getDb(const char *database_name);

    DataBase *getDb(const std::filesystem::path database_path);

    Re delDb(const char *database_name);

    Re delDb(const std::filesystem::path database_path);

    Re closeDb(const char *database_name);

    void destroy();

    std::filesystem::path const getProjectDefaultDatabasePath() { return project_default_database_path_; }

    std::filesystem::path const getProjectBinPath() { return project_bin_path_; }

public:
    static Re createDb(const std::filesystem::path database_path);

private:

    std::filesystem::path project_default_database_path_, project_bin_path_;
    std::map<std::string, DataBase *> opened_databases_;
};

