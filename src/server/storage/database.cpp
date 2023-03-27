#include "database.h"
#include "../parse/parse_defs.h"
#include "../storage/storage_defs.h"

DataBase::DataBase(const char *database_name) {
    database_name_ = std::string(database_name);
}

//Re DataBase::initialize() {
//    // open dfile
//    if (isExists()) {
//        char *const bin_path = GlobalParamsManager::getInstance().getProjectBinaryPath();
//        printf("bin_path:%s\n", bin_path);
//        return Re::SUCCESS;
//    }
//    return Re::FAIL;
//}

//Re DataBase::create() {
//    if (isExists())
//        return Re::FAIL;
//    const char *bin_path = GlobalParamsManager::getInstance().getProjectBinaryPath();
//    return Re::SUCCESS;
//}

void DataBase::destroy() {
}

DataBaseManager &DataBaseManager::getInstance() {
    static DataBaseManager instance;
    return instance;
}

void DataBaseManager::initialize() {}

Re DataBaseManager::createDb(const char *database_name) {
    return Re();
}

void DataBaseManager::destroy() {
}

DataBaseManager::DataBaseManager() {}
