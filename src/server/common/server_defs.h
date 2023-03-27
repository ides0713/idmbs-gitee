#pragma once

#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <filesystem>

const int MAX_CONNECTS = 10;

enum Re {
    Success = 0,
    Fail,
    Error
};

class GlobalParamsManager {
public:
    static GlobalParamsManager &getInstance();

    void initialize();

    std::filesystem::path *getProjectPath() { return project_path_; }

    std::filesystem::path *getProjectBinaryPath() { return project_binary_path_; }

    std::filesystem::path *getProjectBinPath() { return project_bin_path_; }

    void destroy();

private:
    GlobalParamsManager();

    std::filesystem::path *project_path_, *project_binary_path_, *project_bin_path_;
};

