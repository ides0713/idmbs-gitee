#pragma once
#include <cstdio>
#include <dirent.h>
#include <filesystem>
#include <regex>
#include <vector>
/// @brief list all files meet the regular getExpression in the vector providing
int ListFile(std::filesystem::path dir_path, const char *regx, std::vector<std::string> &files_name);
/// @brief get path of database directory path with bin directory path and database name
std::filesystem::path GetDataBasePath(std::filesystem::path bin_path, const char *database_name);
///@brief get path of table meta file's path([table_name].table) with database directory path and table name
std::filesystem::path GetTableMetaFilePath(std::filesystem::path database_path, const char *table_name);
///@brief get path of table data file's path([table_name].data) with database directory path and table name
std::filesystem::path GetTableDataFilePath(std::filesystem::path database_path, const char *table_name);
