#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include "parse/parse_main.h"
#include "storage/storage_main.h"
#include "../src/common_defs.h"
#include "../src/server_defs.h"
#include "../src/global_defs.h"

const int SERVER_PORT = 8888;
const int BUFFER_SIZE = 100;
const int MAX_CONNECTS = 10;

void pStart(const char *sql, int sock_fd)
{
    ParseMain p1;
    RE re_parse = p1.handle(sql);
    if (re_parse != RE::SUCCESS)
    {
        printf("sql parse failed\n");
        return;
    }
    else
    {
        printf("sql parse succeeded\n");
    }
    StorageMain p2(p1.getQuery());
    p2.handle();
}
void recvFunc(int fd)
{
    int n, ret_val;
    message m;
    while ((n = read(fd, reinterpret_cast<char *>(&m), sizeof(m))) > 0)
    {
        if (n == 0)
            break;
        else
        {
            if (m.type_ == MSG_TYPE_EXIT)
            {
                message m(MSG_TYPE_EXIT, "exit");
                write(fd, reinterpret_cast<char *>(&m), sizeof(m));
                break;
            }
            else
            {
                printf("from client:%s\n", m.message_);
                // detach a thread to do sql parsing and other work
                std::thread solve_thread(pStart, m.message_, fd);
                solve_thread.detach();
            }
        }
    }
    printf("disconnect from conn_fd:%d\n", fd);
    close(fd);
}
int main()
{
    GPM.initialize();
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
    printf("buffer content:\n--\n%s\n--\n", buffer);
    pStart(buffer, -1);


    GPM.destroy();
    return 0;
}