#include "storage_defs.h"
#include <cstring>
#include "../../common/common_defs.h"
#include <dirent.h>

std::filesystem::path getDataBasePath(std::filesystem::path bin_path, const char *database_name)
{
    return bin_path.append(database_name);
}

std::filesystem::path getTableMetaFilePath(std::filesystem::path database_path, const char *table_name)
{
    std::string table_meta_file_name = std::string(table_name) + ".table";
    return database_path.append(table_meta_file_name);
}

std::filesystem::path getTableDataFilePath(std::filesystem::path database_path, const char *table_name)
{
    std::string table_data_file_name = std::string(table_name) + ".data";
    return database_path.append(table_data_file_name);
}

int listFile(std::filesystem::path dir_path, const char *regx, std::vector<std::string> &files_name)
{
    int b_size = files_name.size();
    DIR *dir = opendir(dir_path.c_str());
    if (dir == nullptr)
    {
        debugPrint("listFile:open directory failure. path %s, errmsg %d:%s\n", dir_path.c_str(), errno,
                   strerror(errno));
        return -1;
    }
    std::regex express(regx);
    dirent *file;
    while ((file = readdir(dir)) != nullptr)
    {
        if (file->d_type == DT_REG)
        {
            std::string file_name = std::string(file->d_name);
            if (std::regex_match(file_name, express))
                files_name.emplace_back(file->d_name);
        }
    }
    return files_name.size() - b_size;
}
