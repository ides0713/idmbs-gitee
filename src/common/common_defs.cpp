#include "common_defs.h"
#include "params_deliver.h"// for PROJECT_PATH
#include <bits/chrono.h>   // for filesystem
#include <cassert>         // for assert
#include <cmath>
#include <cstdarg>   // for va_end, va_list, va_start
#include <cstdio>    // for fflush, fopen, vfprintf
#include <cstring>   // for strcpy
#include <filesystem>// for path
const double epsilon = 1E-6;
void DebugPrint(const char *format, ...) {
    // #ifdef DEBUG
    if (LOG_STREAM == nullptr) {
        namespace fs = std::filesystem;
        fs::path p(PROJECT_PATH);
        p.append("debug.log");
        LOG_STREAM = fopen(p.c_str(), "a+");
        assert(LOG_STREAM != nullptr);
    }
    va_list var_list;
    // va_start(var_list, format);
    // vprintf(format, var_list);
    // va_end(var_list);
    va_start(var_list, format);
    vfprintf(LOG_STREAM, format, var_list);
    fflush(LOG_STREAM);
    va_end(var_list);
    // #endif
}
bool StrBlank(const char *str) {
    if (str == nullptr)
        return true;
    return strlen(str) == 0;
}
char *SubStr(const char *s, int n_1, int n_2) /*从s中提取下标为n1~n2的字符组成一个新字符串，然后返回这个新串的首地址*/
{
    char *sp = new char[n_2 - n_1 + 2];
    int i, j = 0;
    for (i = n_1; i <= n_2; i++) {
        sp[j++] = s[i];
    }
    sp[j] = 0;
    return sp;
}
int CompareInt(void *arg1, void *arg2) {
    auto a = reinterpret_cast<int *>(arg1), b = reinterpret_cast<int *>(arg2);
    return *a - *b;
}
int CompareFloat(void *arg1, void *arg2) {
    auto a = reinterpret_cast<float *>(arg1), b = reinterpret_cast<float *>(arg2);
    float res = *a - *b;
    if (res > epsilon)
        return 1;
    if (res < epsilon)
        return -1;
    return 0;
}
int CompareString(void *arg1, int arg1_max_length, void *arg2, int arg2_max_length) {
    auto a = reinterpret_cast<char *>(arg1), b = reinterpret_cast<char *>(arg2);
    int max_len = std::min(arg1_max_length, arg2_max_length);
    int res = strncmp(a, b, max_len);
    if (res != 0)
        return res;
    if (arg1_max_length > arg2_max_length)
        return a[max_len] - 0;
    else if (arg1_max_length < arg2_max_length)
        return b[max_len] - 0;
    return 0;
}
