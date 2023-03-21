#pragma once

#include <dirent.h>
#include <stdio.h>

char *getParentDir(char *current_dir);

dirent *findFile(DIR *dir, const char *target_file_name);

dirent *findDir(DIR *dir, const char *target_file_name);