// #include <bits/stdc++.h>
// #include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include "../src/message.h"
#include "parser/yacc_sql.cpp"
const int SERVER_PORT = 8888;
const int BUFFER_SIZE = 100;
const int MAX_CONNECTS = 10;
void send_func(const char *message, int fd)
{
    char buffer[BUFFER_SIZE];
    strcpy(buffer, message);
    write(fd, buffer, BUFFER_SIZE);
}
void recv_func(int fd)
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
            }
        }
    }
    printf("disconnect from conn_fd:%d\n",fd);
    close(fd);
}
int main()
{
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
    //     std::thread recv_thread(recv_func, conn_fd);
    //     recv_thread.detach();
    // }
    char buffer[100];
    strcpy(buffer,"heat on heat on heat off");
    sql_parse(buffer);
    return 0;
}