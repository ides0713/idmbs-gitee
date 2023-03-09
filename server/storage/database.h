#pragma once
#include "../../src/server_defs.h"
#include <stdio.h>
#include <map>
class DataBase
{
public:
    DataBase();
    bool initialize(const char * name);
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
    static DataBaseManager& getInstance();
    void initialize();
    void open();
private:
    DataBaseManager();
    ~DataBaseManager();
    std::map<const char *,DataBase*> opened_databases;
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