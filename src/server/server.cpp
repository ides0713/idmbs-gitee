#include "../common/common_defs.h"
#include "common/global_managers.h"
#include "common/re.h"
#include "common/server_defs.h"
#include "execute/execute_main.h"
#include "parse/parse_main.h"
#include "resolve/resolve_main.h"
#include "storage/storage_main.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

void DoTest();
void ToLower(char *str);
void ToUpper(char *str);
int main(int argc, char *argv[]) {
    GlobalManagers::Init();
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    char str[READ_BUFFER_SIZE];
    while (true) {
        memset(str, 0, 1024);
        scanf("%[^\n]", &str);
        getchar();
        if (strcmp(str, "t") == 0)
            DoTest();
        else if (strcmp(str, "exit;") == 0)
            break;
        else if (strlen(str) != 0)
            gmm.Handle(str);
        DebugPrint("read from buffer:[%s] len:%d\n", str, int(strlen(str)));
    }
    GlobalManagers::Destroy();
    return 0;
}
void DoTest() {
    printf("test program begin ==========\n");
    namespace fs = std::filesystem;
    GlobalParamsManager &gpm = GlobalManagers::GetGlobalParamsManager();
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    fs::path p = gpm.GetProjectPath();
    p.append("test_sqls");
    std::string sql, file_name(p.c_str());
    std::ifstream istream(file_name);
    while (std::getline(istream, sql)) {
        if (sql.substr(0, 2) != "--" and !sql.empty()) {
            printf("%s\n", sql.c_str());
            gmm.Handle(sql.c_str());
        }
    }
    printf("test program end   ==========\n");
    return;
}
void ToLower(char *str) {
    for (int i = 0; i < strlen(str); i++)
        if (str[i] < 'Z' and str[i] > 'A')
            str[i] = str[i] - 'A' + 'a';
}

void ToUpper(char *str) {
    for (int i = 0; i < strlen(str); i++)
        if (str[i] < 'z' and str[i] > 'a')
            str[i] = str[i] - 'a' + 'A';
}
