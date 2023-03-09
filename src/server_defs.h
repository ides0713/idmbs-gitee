#pragma once
#include <unistd.h>
#include <dirent.h>
char *substr(const char *s, int n1, int n2);
enum RE
{
    SUCCESS = 0,
    FAIL,
    ERROR
};