#include "database.h"
#include "../parse/parse_defs.h"
#include "../storage/storage_defs.h"

DataBase::DataBase(const char *database_name) {
    database_name_ = std::string(database_name);
}

//RE DataBase::initialize() {
//    // open dfile
//    if (isExists()) {
//        char *const bin_path = GlobalParamsManager::getInstance().getProjectBinaryPath();
//        printf("bin_path:%s\n", bin_path);
//        return RE::SUCCESS;
//    }
//    return RE::FAIL;
//}

//RE DataBase::create() {
//    if (isExists())
//        return RE::FAIL;
//    const char *bin_path = GlobalParamsManager::getInstance().getProjectBinaryPath();
//    return RE::SUCCESS;
//}

void DataBase::destroy() {
}

DataBaseManager &DataBaseManager::getInstance() {
    static DataBaseManager instance;
    return instance;
}

void DataBaseManager::initialize() {}

RE DataBaseManager::createDB(const char *database_name) {
    return RE();
}

void DataBaseManager::destroy() {
}

DataBaseManager::DataBaseManager() {}
