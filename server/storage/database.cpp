#include "database.h"
#include "../parse/parse_defs.h"
DataBase::DataBase(const char *name)
{
    database_name_=strnew(name);
    database_dfile_=nullptr;
}
void DataBase::initialize(){
    GlobalParamsManager::getInstance().getBinDir();
}
