#include "server_defs.h"
#include "../storage/storage_defs.h"
#include "../../common/params_deliver.h"

GlobalParamsManager &GlobalParamsManager::getInstance() {
    static GlobalParamsManager instance;
    return instance;
}

void GlobalParamsManager::initialize() {
    using path = std::filesystem::path;
    project_path_ = new path(PROJECT_PATH);
    project_binary_path_ = new path(PROJECT_BINARY_PATH);
    project_bin_path_ = new path(std::string(PROJECT_PATH) + "/bin");
    printf("%s\n%s\n%s\n", project_path_->c_str(), project_binary_path_->c_str(), project_bin_path_->c_str());
}

void GlobalParamsManager::destroy() {
    delete[]project_path_;
    delete[]project_binary_path_;
    delete[]project_bin_path_;
}

GlobalParamsManager::GlobalParamsManager() {
    project_path_ = nullptr, project_binary_path_ = nullptr, project_bin_path_ = nullptr;
}
