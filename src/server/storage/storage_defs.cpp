#include "storage_defs.h"
#include "../../common/common_defs.h"// for DebugPrint
#include <cstring>                   // for strerror
#include <dirent.h>                  // for dirent, opendir, readdir, DIR
#include <errno.h>                   // for errno
#include <regex>                     // for regex_match, match_results<>::...
std::filesystem::path GetDataBasePath(std::filesystem::path bin_path, const char *database_name) {
    return bin_path.append(database_name);
}
std::filesystem::path GetTableMetaFilePath(std::filesystem::path database_path, const char *table_name) {
    std::string table_meta_file_name = std::string(table_name) + ".table";
    return database_path.append(table_meta_file_name);
}
std::filesystem::path GetTableDataFilePath(std::filesystem::path database_path, const char *table_name) {
    std::string table_data_file_name = std::string(table_name) + ".data";
    return database_path.append(table_data_file_name);
}
std::filesystem::path GetTableIndexFilePath(std::filesystem::path database_path, const char *table_name,
                                            const char *index_name) {
    std::string table_index_file_name = std::string(table_name) + "-" + std::string(index_name) + ".index";
    return database_path.append(table_index_file_name);
}
int ListFile(std::filesystem::path dir_path, const char *regx, std::vector<std::string> &files_name) {
    int b_size = files_name.size();
    DIR *dir = opendir(dir_path.c_str());
    if (dir == nullptr) {
        DebugPrint("listFile:open directory failure. path %s, errmsg %d:%s\n", dir_path.c_str(), errno,
                   strerror(errno));
        return -1;
    }
    std::regex express(regx);
    dirent *file;
    while ((file = readdir(dir)) != nullptr) {
        if (file->d_type == DT_REG) {
            std::string file_name = std::string(file->d_name);
            if (std::regex_match(file_name, express))
                files_name.emplace_back(file->d_name);
        }
    }
    return files_name.size() - b_size;
}
