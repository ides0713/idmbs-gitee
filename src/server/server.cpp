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

void pStart(const char *sql, int sock_fd);

void recvFunc(int fd);

int main() {
    GlobalManagers::initialize();
    // int listen_fd, conn_fd;
    // sockaddr_in serve_addr;
    // listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    // bzero(&serve_addr, sizeof(serve_addr));
    // serve_addr.sin_family = AF_INET;
    // serve_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // serve_addr.sin_port = htons(SERVER_PORT);
    // bind(listen_fd, (sockaddr *)&serve_addr, sizeof(serve_addr));
    // listen(listen_fd, MAX_CONNECTS);
    // printf("start listening......\n");
    // while (true)
    // {
    //     conn_fd = accept(listen_fd, (sockaddr *)NULL, NULL);
    //     printf("connect from conn_fd:%d\n", conn_fd);
    //     // int n = read(conn_fd, reinterpret_cast<char *>(&l), sizeof(l));
    //     std::thread recv_thread(recvFunc, conn_fd);
    //     recv_thread.detach();
    // }
    char buffer[100];
    strcpy(buffer, "create table t_basic(id int, age int, name char, score float);");
    debugPrint("Main:sql_buffer:\n--\n%s\n--\n", buffer);
    pStart(buffer, -1);
    GlobalManagers::destroy();
    return 0;
}

void pStart(const char *sql, int sock_fd) {
    ParseMain pm;
    Re re_parse = pm.handle(sql);
    if (re_parse != Re::Success) {
        debugPrint("\nMain:parse failed\n");
        return;
    }
    debugPrint("\nMain:sql parse succeeded\n");
    ResolveMain rm(pm.callBack());
    Re re_resolve = rm.handle();
    if (re_resolve != Re::Success) {
        debugPrint("\nMain:resolve stmt failed\n");
        return;
    }
    debugPrint("\nMain:resolve stmt succeeded\n");
    ExecuteMain em(rm.callBack());
    Re re_execute = em.handle();
    if (re_execute != Re::Success) {
        debugPrint("\nMain:execute stmt failed\n");
        return;
    }
    debugPrint("\nMain:execute stmt succeeded\n");
    // StorageMain sm();
    // Re re_storage = sm.handle(rm.getStatement());
    // if (re_storage != Re::SUCCESS)
    // {
    //     printf("storage failed\n");
    //     return;
    // }
}

void recvFunc(int fd) {
    int n, ret_val;
    Message m;
    while ((n = read(fd, reinterpret_cast<char *>(&m), sizeof(m))) > 0) {
        if (n == 0)
            break;
        else {
            if (m.type == MSG_TYPE_EXIT) {
                Message m(MSG_TYPE_EXIT, "exit");
                write(fd, reinterpret_cast<char *>(&m), sizeof(m));
                break;
            } else {
                printf("from client:%s\n", m.message);
                // detach a thread to do sql parsing and other work
                std::thread solve_thread(pStart, m.message, fd);
                solve_thread.detach();
            }
        }
    }
    printf("disconnect from conn_fd:%d\n", fd);
    close(fd);
}