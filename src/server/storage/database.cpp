#include "database.h"
#include "../parse/parse_defs.h"
#include "../storage/storage_defs.h"

DataBase::DataBase(const char *database_name) {
    database_name_ = std::string(database_name);
}

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
