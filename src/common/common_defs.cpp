#include "common_defs.h"
#include "params_deliver.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <filesystem>
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
Message::Message(int type, const char *msg) : type(type) {
    strcpy(message, msg);
}