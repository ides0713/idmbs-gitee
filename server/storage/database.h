#pragma once
#include "../../src/server_defs.h"
#include <stdio.h>
#include <map>
class DataBase
{
public:
    DataBase(const char *database_name);
    RE initialize();
    // if not exists,create it
    RE create();
    // delete the database
    RE destruction();
    // destroy this
    void destroy();
    char *getDBName() { return database_name_; }
    FILE *getDBDFile() { return database_dfile_; }

private:
    bool isExists();
    char *database_name_;
    FILE *database_dfile_;
};

class DataBaseManager
{
public:
    static DataBaseManager &getInstance();
    void initialize();
    RE createDB(const char * database_name);
    RE openDB(const char * databse_name);
    RE delDB(const char * database_name);
    RE closeDB(const char * database_name);
private:
    DataBaseManager();
    ~DataBaseManager();
    std::map<const char *, DataBase *> opened_databases;
};

// class GlobalParamsManager
// {
// public:
//     static GlobalParamsManager &getInstance();
//     void initialize();
//     char * getBinPath(){return bin_dir_path_;}
//     DIR* getBinDir(){return bin_dir_dir_;}
// private:
//     GlobalParamsManager();
//     ~GlobalParamsManager();
//     char *bin_dir_path_;
//     DIR *bin_dir_dir_;
// };