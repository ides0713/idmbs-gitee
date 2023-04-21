#include "server_defs.h"
#include "../../common/params_deliver.h"
#include "../../common/common_defs.h"

void GlobalParamsManager::init()
{
    namespace fs = std::filesystem;
    project_path_ = fs::path(PROJECT_PATH);
    project_binary_path_ = fs::path(PROJECT_BINARY_PATH);
    project_bin_path_ = fs::path(PROJECT_PATH);
    project_bin_path_.append("bin");
    debugPrint("GlobalParamsManager:\nproject_path:%s\nbinary_path:%s\nbin_path:%s\n",
               project_path_.c_str(), project_binary_path_.c_str(), project_bin_path_.c_str());
    if (!std::filesystem::is_directory(project_bin_path_))
        std::filesystem::create_directory(project_bin_path_);
    debugPrint("GlobalParamsManager:initialized done\n");
}

void GlobalParamsManager::destroy()
{
}
