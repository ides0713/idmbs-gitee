#pragma once

#include <dirent.h>
#include <cstdio>
#include <regex>
#include <filesystem>
#include <vector>

/// @brief list all files meet the regular expression in the vector providing
int listFile(std::filesystem::path dir_path, const char *regx, std::vector<std::string> &files_name);

/// @brief get path of database directory path with bin directory path and database name
std::filesystem::path getDataBasePath(std::filesystem::path bin_path, const char *database_name);

///@brief get path of table meta file's path([table_name].table) with database directory path and table name
std::filesystem::path getTableMetaFilePath(std::filesystem::path database_path, const char *table_name);

///@brief get path of table data file's path([table_name].data) with database directory path and table name
std::filesystem::path getTableDataFilePath(std::filesystem::path database_path, const char *table_name);
