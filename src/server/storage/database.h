#pragma once
#include <cstdio>        // for size_t
#include <filesystem>    // for path
#include <map>           // for map
#include <string>        // for string, hash
#include <unordered_map> // for unordered_map
#include "../common/re.h"// for Re
class CLogManager;
class Table;
struct AttrInfo;
#define DATABASE_NAME_MAX_LEN 20
class DataBase
{
public:
    DataBase() = default;
    Re Init(const char *database_name, const std::filesystem::path &database_path);
    void Destroy();
    const char *GetDbName() { return database_name_.c_str(); }
    std::filesystem::path const GetDbPath() { return database_path_; }
    Re CreateTable(const char *table_name, const size_t attr_infos_num, const AttrInfo *attr_infos);
    Re CreateTable(const std::string &table_name, const size_t attr_infos_num, const AttrInfo *attr_infos);
    ///@brief existed table will be opened when database constructed,when table is creating,
    /// it will be added into opened_tables automatically,so all exist valid tables can be find in opened_tables
    Table *GetTable(const std::string &table_name);
    CLogManager *GetCLogManager() { return clog_manager_; }
    int GetOpenedTablesNum() { return opened_tables_.size(); }

private:
    //    Re create();
    Re OpenAllTables();
    Re Destruction();
    //    bool isExists();
    CLogManager *clog_manager_;
    std::string database_name_;
    std::filesystem::path database_path_;
    std::unordered_map<std::string, Table *> opened_tables_;
};
class GlobalDataBaseManager
{
public:
    GlobalDataBaseManager() = default;
    void Init();
    Re CreateDb(const char *database_name);
    DataBase *GetDb(const char *database_name);
    DataBase *GetDb(const std::filesystem::path database_path);
    Re DelDb(const char *database_name);
    Re DelDb(const std::filesystem::path database_path);
    Re CloseDb(const char *database_name);
    void Destroy();
    std::filesystem::path const GetProjectDefaultDatabasePath() { return project_default_database_path_; }
    std::filesystem::path const GetProjectBinPath() { return project_bin_path_; }
    int GetDbNum() { return opened_databases_.size(); }

public:
    static Re CreateDb(const std::filesystem::path database_path);

private:
    std::filesystem::path project_default_database_path_, project_bin_path_;
    std::map<std::string, DataBase *> opened_databases_;

private:
    Re CloseAllDb();
};
