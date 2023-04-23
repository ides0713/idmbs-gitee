#include <bits/chrono.h>                 // for filesystem
#include <cstdio>                        // for printf, getchar, scanf
#include <cstring>                       // for strlen, strcmp, memset
#include <filesystem>                    // for path
#include <fstream>                       // for basic_istream, ifstream
#include <string>                        // for allocator, string, getline

#include "../common/common_defs.h"       // for DebugPrint
#include "common/global_managers.h"      // for GlobalManagers
#include "common/server_defs.h"          // for GlobalParamsManager, READ_BU...
#include "common/global_main_manager.h"  // for GlobalMainManager

const char *EXIT_COMMAND = "exit;";
const char *TEST_COMMAND = "t";
void DoTest();
void ToLower(char *str);
void ToUpper(char *str);
int main(int argc, char *argv[]) {
    //TODO users and corresponding config file
    //TODO test case and tools to generate it
    GlobalManagers::Init();
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    char str[READ_BUFFER_SIZE];
    while (true) {
        printf("[iSQL]: ");
        memset(str, 0, 1024);
        scanf("%[^\n]", &str);
        getchar();
        if (strcmp(str, TEST_COMMAND) == 0)
            DoTest();
        else if (strcmp(str, EXIT_COMMAND) == 0)
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
