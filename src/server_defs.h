#pragma once
#include <unistd.h>
#include <dirent.h>
enum RE
{
    SUCCESS = 0,
    FAIL,
    ERROR
};

class GlobalParamsManager
{
public:
    static GlobalParamsManager &getInstance();
    void initialize();
    char * getBinPath(){return bin_dir_path_;}
    DIR* getBinDir(){return bin_dir_dir_;}
private:
    GlobalParamsManager();
    ~GlobalParamsManager();
    char *bin_dir_path_;
    DIR *bin_dir_dir_;
};
// class Singleton
// {
// private:
// 	Singleton() { };
// 	~Singleton() { };
// 	Singleton(const Singleton&);
// 	Singleton& operator=(const Singleton&);
// public:
// 	static Singleton& getInstance()
//         {
// 		static Singleton instance;
// 		return instance;
// 	}
// };

// #pragma once
// #include <unistd.h>
// #include <dirent.h>
// #include "common_defs.h"
// #include "server_defs.h"
// #include <sys/stat.h>
// #include <sys/types.h>
// #include "../server/parse/parse.h"
// #include "../server/storage/storage_defs.h"
// class GlobalParamsManager
// {
// public:
//     GlobalParamsManager();
//     void initialize();
//     void destroy();
//     char *getBinDirPath() { return bin_dir_path_; }
//     DIR *getBinDirDir() { return bin_dir_dir_; }

// private:
//     char *bin_dir_path_;
//     DIR *bin_dir_dir_;
// } GPM;

// GlobalParamsManager::GlobalParamsManager()
// {
//     bin_dir_dir_ = nullptr;
//     bin_dir_path_ = nullptr;
// }

// void GlobalParamsManager::initialize()
// {
//     char *parent_str = new char[DIR_PATH_LEN];
//     getcwd(parent_str, DIR_PATH_LEN);
//     parent_str = getParentDir(parent_str);
//     DIR *parent_dir = opendir(parent_str);
//     dirent *a = findDir(parent_dir, "bin");
//     strcat(parent_str, "/bin");
//     if (a == nullptr)
//         mkdir(parent_str, 0777);
//     bin_dir_path_ = strnew(parent_str);
//     closedir(parent_dir);
//     bin_dir_dir_ = opendir(bin_dir_path_);
//     delete[] parent_str;
// }

// void GlobalParamsManager::destroy()
// {
//     closedir(bin_dir_dir_);
//     delete[] bin_dir_path_;
// }
