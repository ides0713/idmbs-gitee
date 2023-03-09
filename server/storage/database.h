#pragma once
#include "../../src/common_defs.h"
class DataBase
{
public:
    DataBase(const char *name);

private:
    char *database_name_;
};
class DataBaseManager
{
public:
    DataBaseManager();

private:
    DataBase *opened_databases_;
};