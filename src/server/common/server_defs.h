#pragma once

#include <dirent.h>
#include <filesystem>
#include <unistd.h>
#include <vector>

const int MAX_CONNECTS = 10;

const int READ_BUFFER_SIZE = 1024;
class GlobalParamsManager {
public:
    GlobalParamsManager() = default;

    void Init();

    std::filesystem::path const GetProjectPath() { return project_path_; }

    std::filesystem::path const GetProjectBinaryPath() { return project_binary_path_; }

    std::filesystem::path const GetProjectBinPath() { return project_bin_path_; }

    void Destroy();

private:
    std::filesystem::path project_path_, project_binary_path_, project_bin_path_;
};
