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

class DataBase {
public:
    explicit DataBase(const char *database_name);

    Re initialize();

    void destroy();

    const std::string getDbName() { return database_name_; }

    std::filesystem::path const getDbPath() { return database_path_; }

    Re createTable(const char *table_name, const size_t attr_infos_num, const AttrInfo *attr_infos);

    Re createTable(std::string table_name, const size_t attr_infos_num, const AttrInfo *attr_infos);

    const Table *getTable();

private:
//    Re create();

    Re destruction();

//    bool isExists();

    std::string database_name_;
    std::filesystem::path database_path_;
    std::unordered_map<std::string, Table *> opened_tables_;
};


class DataBaseManager {
public:
    DataBaseManager();

    void initialize();

    Re createDb(const char *database_name);

    Re createDb(const std::filesystem::path database_path);

    DataBase *getDb(const char *database_name);

    DataBase *getDb(const std::filesystem::path database_path);

    Re delDb(const char *database_name);

    Re delDb(const std::filesystem::path database_path);

    Re closeDb(const char *database_name);

    void destroy();

    std::filesystem::path const getProjectDefaultDatabasePath() { return project_default_database_path_; }

    std::filesystem::path const getProjectBinPath() { return project_bin_path_; }

private:
    std::filesystem::path project_default_database_path_, project_bin_path_;
    std::map<std::string, DataBase *> opened_databases_;
};

