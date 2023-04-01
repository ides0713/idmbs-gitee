#include "database.h"
#include "../../common/common_defs.h"
#include "../common/global_managers.h"

DataBase::DataBase(const char *database_name) {
    namespace fs = std::filesystem;
    database_name_ = std::string(database_name);
    GlobalParamsManager gpm = GlobalManagers::globalParamsManager();
    database_path_ = fs::path(gpm.getProjectBinPath()).append(database_name);
    clog_manager_ = nullptr;
}

void DataBase::destroy() {
}

Re DataBase::createTable(const char *table_name, const size_t attr_infos_num, const AttrInfo *attr_infos) {
    namespace fs = std::filesystem;
    //todo:open all existed tables when DataBase was constructed
    if (opened_tables_.count(std::string(table_name))) {
        debugPrint("DataBase::create table %s in database %s failed,already exists\n", database_name_.c_str(),
                   table_name);
        return Re::Fail;
    }
    GlobalDataBaseManager dbm = GlobalManagers::globalDataBaseManager();
    std::string table_file_name = std::string(table_name).append(".table");
    fs::path table_file_path = fs::path(database_path_).append(table_file_name);
    Table *new_table = new Table();
    new_table->initialize(database_path_, table_name, attr_infos_num, attr_infos);
    return Re::Fail;
}

Re DataBase::createTable(const std::string &table_name, const size_t attr_infos_num, const AttrInfo *attr_infos) {
    return createTable(table_name.c_str(), attr_infos_num, attr_infos);
}

Re DataBase::initialize() {
    //todo:initialize clog_manager of the database(not implement)
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
    if (!fs::is_directory(directory_path)) {
        bool res = fs::create_directory(directory_path);
        if (res) {
            debugPrint("DatabaseManager:created database %s\n", database_name);
            return Re::Success;
        } else {
            debugPrint("DatabaseManager:create database %s failed,unknown error\n", database_name);
            return Re::Fail;
        }
    } else {
        debugPrint("DatabaseManager:create database %s failed,already exists\n", database_name);
        return Re::Fail;
    }
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
            return Re::Fail;
        }
    } else {
        debugPrint("DatabaseManager:create database %s failed,already exists\n", database_name.c_str());
        return Re::Fail;
    }
}

void GlobalDataBaseManager::destroy() {
    for (auto it: opened_databases_) {
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
            DataBase *th_new_db = new DataBase(database_name);
            Re r = th_new_db->initialize();
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
