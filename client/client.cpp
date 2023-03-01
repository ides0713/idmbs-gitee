#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <boost/thread.hpp>
#include "../src/common_defs.h"
const int SERVER_PORT = 8888;
const int BUFFER_SIZE = 100;
void recvFunc(int fd)
{

    int n;
    message m;
    while ((n = read(fd, reinterpret_cast<char *>(&m), sizeof(m))) > 0)
    {
        if (m.type_ == MSG_TYPE_EXIT)
        {
            printf("exit success\n");
            break;
        }
        else
            printf("recv_message:%s\n", m.message_);
    }
    close(fd);
}
int main(int argc, char **argv)
{
    int sock_fd, n;
    char buffer[BUFFER_SIZE];
    sockaddr_in server_addr;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return EXIT_FAILURE;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
        return EXIT_FAILURE;
    if (connect(sock_fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        return EXIT_FAILURE;
    std::thread recv_thread(recvFunc, sock_fd);
    recv_thread.detach();
    while (true)
    {
        printf("client:");
        std::cin.getline(buffer, BUFFER_SIZE);
        if (strcmp(buffer, "") != 0)
        {
            if (strcmp(buffer, "exit") == 0 or strcmp(buffer, "EXIT") == 0)
            {
                message m(MSG_TYPE_EXIT, "exit");
                write(sock_fd, reinterpret_cast<char *>(&m), sizeof(m));
                break;
            }
            else
            {
                message m(MSG_TYPE_REQUEST, "");
                strcpy(m.message_, buffer);
                write(sock_fd, reinterpret_cast<char *>(&m), sizeof(m));
            }
        }
    }
    return EXIT_SUCCESS;
}