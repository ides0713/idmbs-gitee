#pragma once
#include "../../src/server_defs.h"
#include <stdio.h>
class DataBase
{
public:
    DataBase(const char *name);
    void initialize();
    //if not exists,create it 
    void create();
    //delete the database
    void destruction();
    //destroy this
    void destroy();
private:
    bool ifExists();
    char *database_name_;
    FILE* database_dfile_;
};
class DataBaseManager
{
public:
    DataBaseManager();

private:
    DataBase *opened_databases_;
};