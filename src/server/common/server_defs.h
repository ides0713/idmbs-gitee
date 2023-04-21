#pragma once

#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <filesystem>

const int MAX_CONNECTS = 10;

const int READ_BUFFER_SIZE = 1024;
class GlobalParamsManager
{
public:
    GlobalParamsManager() = default;

    void init();

    std::filesystem::path const getProjectPath() { return project_path_; }

    std::filesystem::path const getProjectBinaryPath() { return project_binary_path_; }

    std::filesystem::path const getProjectBinPath() { return project_bin_path_; }

    void destroy();

private:
    std::filesystem::path project_path_, project_binary_path_, project_bin_path_;
};
