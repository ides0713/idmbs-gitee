#pragma once
#include <unistd.h>
#include <dirent.h>
#include <vector>
const int MAX_CONNECTS = 10;

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
    char *getBinPath() { return bin_dir_path_; }
    DIR *getBinDir() { return bin_dir_dir_; }

private:
    GlobalParamsManager();
    ~GlobalParamsManager();
    char *bin_dir_path_;
    DIR *bin_dir_dir_;
};

