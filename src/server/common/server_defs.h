#pragma once

#include <unistd.h>
#include <dirent.h>
#include <vector>

const int MAX_CONNECTS = 10;

enum RE {
    SUCCESS = 0,
    FAIL,
    ERROR
};

class GlobalParamsManager {
public:
    static GlobalParamsManager &getInstance();

    void initialize();

    char *getProjectPath() { return project_path_; }

    char *getProjectBinaryPath() { return project_binary_path_; }

    void destroy();

private:
    GlobalParamsManager();

    char *project_path_, *project_binary_path_,*project_bin_path_;
};

