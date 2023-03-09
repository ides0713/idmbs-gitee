#include "database.h"
#include "../parse/parse_defs.h"
DataBase::DataBase()
{
    database_name_ = nullptr;
    database_dfile_ = nullptr;
}
bool DataBase::initialize(const char * name)
{
    GlobalParamsManager::getInstance().getBinDir();
}

void DataBase::create()
{

}

DataBaseManager &DataBaseManager::getInstance()
{
    static DataBaseManager instance;
    return instance;
}


