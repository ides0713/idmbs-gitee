#include "../common/common_defs.h"     // for DebugPrint
#include "common/global_main_manager.h"// for GlobalMainManager
#include "common/global_managers.h"    // for GlobalManagers
#include "common/server_defs.h"        // for GlobalParamsManager, READ_BU...
#include <assert.h>
#include <bits/chrono.h>// for filesystem
#include <cstdio>       // for printf, getchar, scanf
#include <cstring>      // for strlen, strcmp, memset
#include <filesystem>   // for path
#include <fstream>      // for basic_istream, ifstream
#include <string>       // for allocator, string, getline
#include <unistd.h>
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
        if (strcmp(str, TEST_COMMAND) == 0){
            DoTest();
            break;
        }
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
    namespace fs = std::filesystem;
    GlobalParamsManager &gpm = GlobalManagers::GetGlobalParamsManager();
    GlobalMainManager &gmm = GlobalManagers::GetGlobalMainManager();
    fs::path project_path = gpm.GetProjectPath();
    fs::path test_sqls_path = fs::path(project_path).append("test_sqls");
    fs::path test_results_path = fs::path(project_path).append("test_results");
    FILE *fp = fopen(test_results_path.c_str(), "w+");
    int fd = fileno(fp), oldfd = dup(STDOUT_FILENO);
    dup2(fd, STDOUT_FILENO);
    printf("test program begin ==========\n");
    std::string sql, sqls_file_name(test_sqls_path.c_str());
    std::ifstream istream(sqls_file_name);
    int i = 0;
    while (std::getline(istream, sql)) {
        if (sql.substr(0, 2) != "--" and !sql.empty()) {
            printf("[SQL %d]:%s\n", ++i, sql.c_str());
            gmm.Handle(sql.c_str());
        }
    }
    printf("test program end   ==========");
    //todo:resume
    fflush(stdout);
    dup2(oldfd, STDOUT_FILENO);
    fclose(fp);
    printf("test done\n");
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
