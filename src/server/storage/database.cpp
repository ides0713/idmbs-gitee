#include "database.h"
#include "../common/global_managers.h"
#include "storage_defs.h"
#include "storage_handler.h"

void DataBase::destroy() {
}

Re DataBase::initialize(const char *database_name, const std::filesystem::path &database_path) {
    //todo:init clog_manager of the database(not implement)
    namespace fs = std::filesystem;
    if (strlen(database_name) == 0) {
        debugPrint("DataBase:failed to init DB, name invalid:%s.\n", database_name);
        return Re::InvalidArgument;
    }
    if (!fs::is_directory(database_path)) {
        debugPrint("DataBase:failed to init DB, path is not a directory: %s\n", database_path.c_str());
        return Re::GenericError;
    }
    clog_manager_ = new ClogManager(database_path);
    if (clog_manager_ == nullptr) {
        debugPrint("DataBase:failed to init CLogManager.\n");
        return Re::GenericError;
    }
    database_name_ = std::string(database_name);
    database_path_ = database_path;
    return openAllTables();
}

Re DataBase::createTable(const char *table_name, const size_t attr_infos_num, const AttrInfo *attr_infos) {
    namespace fs = std::filesystem;
    if (opened_tables_.count(std::string(table_name))) {
        debugPrint("DataBase::create table %s in database %s failed,already exists\n", database_name_.c_str(),
                   table_name);
        return Re::SchemaTableExist;
    }
    Table *new_table = new Table();
    //todo:table.init causes segmentation fault
    Re r = new_table->init(database_path_, table_name, attr_infos_num, attr_infos, clog_manager_);
    if (r != Re::Success) {
        debugPrint("DataBase:failed to create table %s.\n", table_name);
        delete new_table;
        return r;
    }
    opened_tables_[table_name] = new_table;
    debugPrint("DataBase:create table success. table name=%s\n", table_name);
    return Re::Success;
}

Re DataBase::createTable(const std::string &table_name, const size_t attr_infos_num, const AttrInfo *attr_infos) {
    return createTable(table_name.c_str(), attr_infos_num, attr_infos);
}

Re DataBase::openAllTables() {
    std::vector<std::string> table_meta_files;
    std::string regx_table_meta_file_name = "^\\w*\\.(table)$";
    int ret = listFile(database_path_, regx_table_meta_file_name.c_str(), table_meta_files);
    if (ret < 0) {
        debugPrint("DataBase:failed to list table meta files under %s.\n", database_path_.c_str());
        return Re::IoErr;
    }
    for (const std::string &file_name: table_meta_files) {
        Table *table = new Table();
        Re r = table->init(database_path_, file_name.c_str(), clog_manager_);
        if (r != Re::Success) {
            delete table;
            debugPrint("DataBase:failed to open table. file_name=%s\n", file_name.c_str());
            return r;
        }
        if (opened_tables_.count(table->getTableName())) {
            delete table;
            debugPrint("DataBase:duplicate table with difference file name. table=%s, the other file_name=%s\n",
                       table->getTableName().c_str(),
                       file_name.c_str());
            return Re::GenericError;
        }
        opened_tables_[table->getTableName()] = table;
        debugPrint("DataBase:open table: %s, file: %s\n", table->getTableName().c_str(), file_name.c_str());
    }
    debugPrint("DataBase:all table have been opened. num=%d\n", opened_tables_.size());
    return Re::Success;
}

void GlobalDataBaseManager::initialize() {
    namespace fs = std::filesystem;
    GlobalParamsManager gpm = GlobalManagers::globalParamsManager();
    project_default_database_path_ = fs::path(
            gpm.getProjectBinPath()).append("sys");
    if (!std::filesystem::is_directory(project_default_database_path_))
        std::filesystem::create_directory(project_default_database_path_);
    debugPrint("default database path is %s\n", project_default_database_path_.c_str());
    project_bin_path_ = fs::path(project_default_database_path_.parent_path());
    debugPrint("bin path is %s\n", project_bin_path_.c_str());
    debugPrint("GlobalDataBaseManager:initialized done\n");
}

Re GlobalDataBaseManager::createDb(const char *database_name) {
    namespace fs = std::filesystem;
    fs::path directory_path = fs::path(project_bin_path_.c_str()).append(database_name);
    return createDb(directory_path);
}

Re GlobalDataBaseManager::createDb(const std::filesystem::path database_path) {
    namespace fs = std::filesystem;
    std::string database_name = std::string(database_path.filename().c_str());
    if (!fs::is_directory(database_path)) {
        bool res = fs::create_directory(database_path);
        if (res) {
            debugPrint("DatabaseManager:created database %s\n", database_name.c_str());
            return Re::Success;
        } else {
            debugPrint("DatabaseManager:create database %s failed,unknown error\n", database_name.c_str());
            return Re::GenericError;
        }
    } else {
        debugPrint("DatabaseManager:create database %s failed,already exists\n", database_name.c_str());
        return Re::SchemaDbExist;
    }
}

void GlobalDataBaseManager::destroy() {
    for (const auto &it: opened_databases_) {
        opened_databases_.erase(it.first);
        delete it.second;
    }
}


DataBase *GlobalDataBaseManager::getDb(const char *database_name) {
    std::string th_database_name = std::string(database_name);
    if (opened_databases_.count(th_database_name)) {
        debugPrint("GlobalDataBaseManager:getDb from opened_databases,database_name:%s\n", database_name);
        return opened_databases_[th_database_name];
    } else {
        namespace fs = std::filesystem;
        fs::path th_database_path = project_bin_path_.append(database_name);
        if (fs::is_directory(th_database_path)) {
            DataBase *th_new_db = new DataBase();
            Re r = th_new_db->initialize(database_name, th_database_path);
            if (r == Re::Success) {
                opened_databases_.emplace(std::string(database_name), th_new_db);
                debugPrint("GlobalDataBaseManager:getDb succeeded,database_name:%s\n", database_name);
                return th_new_db;
            } else {
                debugPrint("GlobalDataBaseManager:getDb failed,create failed,database_name:%s\n", database_name);
                return nullptr;
            }
        } else {
            debugPrint("GlobalDataBaseManager:getDb failed,database %s not exists\n", database_name);
            return nullptr;
        }
    }
}

DataBase *GlobalDataBaseManager::getDb(const std::filesystem::path database_path) {
    std::string th_database_name = std::string(database_path.filename());
    return getDb(th_database_name.c_str());
}
