#include "server_defs.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "../../common/common_defs.h"
#include "../parse/parse_defs.h"
#include "../storage/storage_defs.h"
#include "../../common/params_deliver.h"
#include <filesystem>
#include <cstring>
namespace fs=std::filesystem;
GlobalParamsManager &GlobalParamsManager::getInstance() {
    static GlobalParamsManager instance;
    return instance;
}

void GlobalParamsManager::initialize() {
    project_path_ = strnew(PROJECT_PATH);
    project_binary_path_ = strnew(PROJECT_BINARY_PATH);
    project_bin_path_=strnew(project_path_);
    strcat(project_bin_path_,"/bin");
    printf("%s\n%s\n%s\n",project_path_,project_binary_path_,project_bin_path_);
}

void GlobalParamsManager::destroy() {
    delete[]project_path_;
    delete[]project_binary_path_;
    delete[]project_bin_path_;
}

GlobalParamsManager::GlobalParamsManager() {
    project_path_ = nullptr, project_binary_path_ = nullptr;
}
