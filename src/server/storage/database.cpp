#include "database.h"
#include "../../common/common_defs.h"// for DebugP...
#include "../common/global_main_manager.h"
#include "../common/global_managers.h"// for Global...
#include "../common/server_defs.h"    // for Global...
#include "clog_manager.h"             // for CLogMa...
#include "storage_defs.h"             // for ListFile
#include "table.h"                    // for Table
#include <bits/chrono.h>              // for filesy...
#include <cstring>                    // for strlen
#include <utility>                    // for pair
#include <vector>                     // for vector
struct AttrInfo;
void DataBase::Destroy() {
    // destroy all table of the database,remove them from the memory and
    for (auto table: opened_tables_)
        table.second->Destroy();
    opened_tables_.clear();
    if (clog_manager_ != nullptr) {
        delete clog_manager_;
        clog_manager_ = nullptr;
    }
}
Re DataBase::Init(const char *database_name, const std::filesystem::path &database_path) {
    namespace fs = std::filesystem;
    if (StrBlank(database_name)) {
        DebugPrint("DataBase:failed to init DB, name invalid:%s.\n", database_name);
        return Re::InvalidArgument;
    }
    if (!fs::is_directory(database_path)) {
        DebugPrint("DataBase:failed to init DB, path is not a directory: %s\n", database_path.c_str());
        return Re::GenericError;
    }
    clog_manager_ = new CLogManager(database_path.c_str());
    if (clog_manager_ == nullptr) {
        DebugPrint("DataBase:failed to init CLogManager.\n");
        return Re::GenericError;
    }
    database_name_ = std::string(database_name);
    database_path_ = database_path;
    return OpenAllTables();
}
Re DataBase::CreateTable(const char *table_name, const size_t attr_infos_num, const AttrInfo *attr_infos) {
    namespace fs = std::filesystem;
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    if (opened_tables_.count(std::string(table_name))) {
        DebugPrint("DataBase::create table %s in database %s failed,already exists\n", table_name,
                   database_name_.c_str());
        gmm.SetResponse("CREATE TABLE '%s' FAILED,TABLE ALREADY EXISTS.\n", table_name);
        return Re::SchemaTableExist;
    }
    Table *new_table = new Table;
    Re r = new_table->Init(database_path_, table_name, attr_infos_num, attr_infos, clog_manager_);
    if (r != Re::Success) {
        DebugPrint("DataBase:failed to createFilter table %s.\n", table_name);
        delete new_table;
        if (r == Re::SchemaTableExist)
            gmm.SetResponse("CREATE TABLE '%s' FAILED,TABLE ALREADY EXISTS.\n", table_name);
        else if (r == Re::InvalidArgument)
            gmm.SetResponse("CREATE TABLE '%s' FAILED,ARGUMENT IS INVALID.\n", table_name);
        return r;
    }
    opened_tables_[table_name] = new_table;
    DebugPrint("DataBase:create table succeeded. table name=%s\n", table_name);
    return Re::Success;
}
Re DataBase::CreateTable(const std::string &table_name, const size_t attr_infos_num, const AttrInfo *attr_infos) {
    return CreateTable(table_name.c_str(), attr_infos_num, attr_infos);
}
Re DataBase::DropTable(const char *table_name) {
    auto it = opened_tables_.find(table_name);
    if (it == opened_tables_.end()){
        DebugPrint("Database:drop table failed,can not find table %s\n",table_name);
        return Re::SchemaTableNotExist;
        }// 找不到表，要返回错误，测试程序中也会校验这种场景
    Table *table = it->second;
    Re r = table->Drop();// 让表自己销毁资源
    if (r != Re::Success)
        return r;
    opened_tables_.erase(it);// 删除成功的话，从表list中将它删除
    delete table;
    return Re::Success;
}
Re DataBase::OpenAllTables() {
    std::vector<std::string> table_meta_files;
    std::string regx_table_meta_file_name = "^\\w*\\.(table)$";
    int ret = ListFile(database_path_, regx_table_meta_file_name.c_str(), table_meta_files);
    if (ret < 0) {
        DebugPrint("DataBase:failed to list table meta files under %s.\n", database_path_.c_str());
        return Re::IoErr;
    }
    for (const std::string &file_name: table_meta_files) {
        Table *table = new Table;
        std::string table_name = file_name.substr(0, file_name.find_last_of('.'));
        Re r = table->Init(database_path_, table_name.c_str(), clog_manager_);
        if (r != Re::Success) {
            delete table;
            DebugPrint("DataBase:failed to open table. file_name=%s\n", file_name.c_str());
            return r;
        }
        if (opened_tables_.count(table->GetTableName())) {
            delete table;
            DebugPrint("DataBase:duplicate table with difference file name. table=%s, the other file_name=%s\n",
                       table->GetTableName(), file_name.c_str());
            return Re::GenericError;
        }
        opened_tables_[table->GetTableName()] = table;
        DebugPrint("DataBase:open table: %s, file: %s\n", table->GetTableName(), file_name.c_str());
    }
    DebugPrint("DataBase:all table have been opened. num=%d\n", opened_tables_.size());
    return Re::Success;
}
Table *DataBase::GetTable(const std::string &table_name) {
    auto it = opened_tables_.find(table_name);
    if (it != opened_tables_.end())
        return it->second;
    return nullptr;
}
void GlobalDataBaseManager::Init() {
    namespace fs = std::filesystem;
    GlobalParamsManager &gpm = GlobalManagers::GetGlobalParamsManager();
    project_default_database_path_.assign(gpm.GetProjectBinPath());
    project_default_database_path_.append("sys");
    if (!std::filesystem::is_directory(project_default_database_path_))
        std::filesystem::create_directory(project_default_database_path_);
    DebugPrint("default database path is %s\n", project_default_database_path_.c_str());
    project_bin_path_.assign(gpm.GetProjectBinPath());
    DebugPrint("bin path is %s\n", project_bin_path_.c_str());
    DebugPrint("GlobalDataBaseManager:initialized done\n");
}
Re GlobalDataBaseManager::CreateDb(const char *database_name) {
    namespace fs = std::filesystem;
    fs::path directory_path = fs::path(project_bin_path_.c_str()).append(database_name);
    return CreateDb(directory_path);
}
Re GlobalDataBaseManager::CreateDb(const std::filesystem::path database_path) {
    namespace fs = std::filesystem;
    std::string database_name = std::string(database_path.filename().c_str());
    if (!fs::is_directory(database_path)) {
        bool res = fs::create_directory(database_path);
        if (res) {
            DebugPrint("DatabaseManager:created database %s\n", database_name.c_str());
            return Re::Success;
        } else {
            DebugPrint("DatabaseManager:createFilter database %s failed,unknown error\n", database_name.c_str());
            return Re::GenericError;
        }
    } else {
        DebugPrint("DatabaseManager:createFilter database %s failed,already exists\n", database_name.c_str());
        return Re::SchemaDbExist;
    }
}
Re GlobalDataBaseManager::CloseAllDb() {
    for (const auto &it: opened_databases_) {
        it.second->Destroy();
    }
    opened_databases_.clear();
    return Re::Success;
}
void GlobalDataBaseManager::Destroy() {
    CloseAllDb();
}
DataBase *GlobalDataBaseManager::GetDb(const char *database_name) {
    std::string th_database_name = std::string(database_name);
    if (opened_databases_.count(th_database_name)) {
        DebugPrint("GlobalDataBaseManager:get database from opened_databases,database_name:%s\n", database_name);
        return opened_databases_[th_database_name];
    } else {
        namespace fs = std::filesystem;
        fs::path th_database_path = project_bin_path_.append(database_name);
        if (fs::is_directory(th_database_path)) {
            DataBase *th_new_db = new DataBase();
            Re r = th_new_db->Init(database_name, th_database_path);
            if (r == Re::Success) {
                opened_databases_.emplace(std::string(database_name), th_new_db);
                DebugPrint("GlobalDataBaseManager:get database succeeded,database_name:%s\n", database_name);
                return th_new_db;
            } else {
                DebugPrint("GlobalDataBaseManager:get database failed,createFilter failed,database_name:%s\n",
                           database_name);
                return nullptr;
            }
        } else {
            DebugPrint("GlobalDataBaseManager:get database failed,database %s not exists\n", database_name);
            return nullptr;
        }
    }
}
DataBase *GlobalDataBaseManager::GetDb(const std::filesystem::path database_path) {
    std::string th_database_name = std::string(database_path.filename());
    return GetDb(th_database_name.c_str());
}
