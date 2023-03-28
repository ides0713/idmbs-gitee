#include "storage_main.h"
#include "storage_defs.h"
#include <sys/stat.h>
#include "../../common/common_defs.h"

Re StorageMain::handle(Statement *stmt) {
    DIR *bin_dir = findBin();
    if (bin_dir == nullptr)
        debugPrint("StorageMain:get bin dir failed\n");
    closedir(bin_dir);
    return Re::Fail;
}

StorageMain::StorageMain() = default;

DIR *StorageMain::findBin() {
    char *parent_str = new char[100];
    getcwd(parent_str, 100);
    parent_str = getParentDir(parent_str);
    DIR *parent_dir = opendir(parent_str);
    dirent *a = findDir(parent_dir, "bin");
    if (a == nullptr)
        mkdir(strcat(parent_str, "/bin"), 0777);
    closedir(parent_dir);
    DIR *bin_dir = opendir(strcat(parent_str, "/bin"));
    delete[] parent_str;
    return bin_dir;
}

FILE *StorageMain::findDbFile(DIR *bin_dir) { return nullptr; }