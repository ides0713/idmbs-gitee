#include "storage_defs.h"
#include <string.h>
#include "../../src/server_defs.h"
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

char *getParentDir(char *current_dir)
{
    int len = strlen(current_dir);
    if (len == 1)
        return current_dir;
    int i;
    for (i = len - 1; i >= 0 and current_dir[i] != '/'; i--)
        ;
    return substr(current_dir, 0, i - 1);
}

dirent *findFile(DIR *dir, const char *target_file_name)
{
    dirent *res;
    while ((res = readdir(dir)) != nullptr)
        if (strcmp(res->d_name, target_file_name) == 0 and res->d_type == DT_FIFO)
            return res;
    return res;
}

dirent *findDir(DIR *dir, const char *target_dir_name)
{
    dirent *res;
    while ((res = readdir(dir)) != nullptr)
        if (strcmp(res->d_name, target_dir_name) == 0 and res->d_type == DT_DIR)
            return res;
    return res;
}