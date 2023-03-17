#include "server_defs.h"
#include <sys/stat.h>
#include <sys/types.h>
#include "../../common/common_defs.h"
#include "../parse/parse_defs.h"
#include "../storage/storage_defs.h"

GlobalParamsManager &GlobalParamsManager::getInstance()
{
    static GlobalParamsManager instance;
    return instance;
}

void GlobalParamsManager::initialize()
{
    char *parent_str = new char[DIR_PATH_LEN];
    getcwd(parent_str, DIR_PATH_LEN);
    parent_str = getParentDir(parent_str);
    DIR *parent_dir = opendir(parent_str);
    dirent *a = findDir(parent_dir, "bin");
    strcat(parent_str, "/bin");
    if (a == nullptr)
        mkdir(parent_str, 0777);
    bin_dir_path_ = strnew(parent_str);
    closedir(parent_dir);
    bin_dir_dir_ = opendir(bin_dir_path_);
    delete[] parent_str;
}

void GlobalParamsManager::destroy()
{
}

GlobalParamsManager::GlobalParamsManager()
{
    bin_dir_dir_ = nullptr;
    bin_dir_path_ = nullptr;
}

GlobalParamsManager::~GlobalParamsManager()
{
    closedir(bin_dir_dir_);
    delete[] bin_dir_path_;
}
