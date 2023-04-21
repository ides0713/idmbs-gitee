#include <sys/socket.h>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <cstdio>
#include "../common/common_defs.h"
#include "common/server_defs.h"
#include "common/global_managers.h"
#include "parse/parse_main.h"
#include "resolve/resolve_main.h"
#include "execute/execute_main.h"
#include "storage/storage_main.h"
#include "common/re.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>

void pStart(const char *sql, int sock_fd);

void recvFunc(int fd);

void doTest();
void toLower(char *str);
void toUpper(char *str);
int main(int argc, char *argv[])
{
    GlobalManagers::init();
    GlobalMainManager &gmm = GlobalManagers::globalMainManager();
    char str[READ_BUFFER_SIZE];
    while (true)
    {
        memset(str, 0, 1024);
        scanf("%[^\n]", &str);
        getchar();
        toLower(str);
        if (strcmp(str, "t") == 0)
            doTest();
        else if (strcmp(str, "exit;") == 0)
            break;
        else if (strlen(str) != 0)
            gmm.handle(str);
        debugPrint("read from buffer:[%s] len:%d\n", str, int(strlen(str)));
    }
    GlobalManagers::destroy();
    return 0;
}
void doTest()
{
    printf("test program begin ==========\n");
    namespace fs = std::filesystem;
    GlobalParamsManager &gpm = GlobalManagers::globalParamsManager();
    GlobalMainManager &gmm = GlobalManagers::globalMainManager();
    fs::path p = gpm.getProjectPath();
    p.append("test_sqls");
    std::string sql, file_name(p.c_str());
    std::ifstream istream(file_name);
    while (std::getline(istream, sql))
    {
        if (sql.substr(0, 2) != "--" and !sql.empty())
            gmm.handle(sql.c_str());
    }
    printf("test program end   ==========\n");
    return;
}
void toLower(char *str)
{
    for (int i = 0; i < strlen(str); i++)
        if (str[i] < 'Z' and str[i] > 'A')
            str[i] = str[i] - 'A' + 'a';
}

void toUpper(char *str)
{
    for (int i = 0; i < strlen(str); i++)
        if (str[i] < 'z' and str[i] > 'a')
            str[i] = str[i] - 'a' + 'A';
}
