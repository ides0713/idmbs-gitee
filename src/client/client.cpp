#include <arpa/inet.h>              // for inet_pton
#include <netinet/in.h>             // for sockaddr_in, htons
#include <strings.h>                // for bzero
#include <sys/socket.h>             // for AF_INET, connect, socket, SOCK_ST...
#include <unistd.h>                 // for write, close, read
#include <stdio.h>                  // for printf
#include <stdlib.h>                 // for EXIT_FAILURE, EXIT_SUCCESS
#include <cstring>                  // for strcmp, strcpy
#include <iostream>                 // for cin, istream
#include <thread>                   // for thread

#include "../common/common_defs.h"  // for Message, BUFFER_SIZE, MSG_TYPE_EXIT

void RecvFunc(int fd) {
    Message m;
    while (read(fd, reinterpret_cast<char *>(&m), sizeof(m)) > 0) {
        if (m.type == MSG_TYPE_EXIT) {
            printf("exit success\n");
            break;
        } else
            printf("recv_message:%s\n", m.message);
    }
    close(fd);
}
int main(int argc, char **argv) {
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
    if (connect(sock_fd, (sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        return EXIT_FAILURE;
    std::thread recv_thread(RecvFunc, sock_fd);
    recv_thread.detach();
    while (true) {
        printf("client:");
        std::cin.getline(buffer, BUFFER_SIZE);
        if (strcmp(buffer, "") != 0) {
            if (strcmp(buffer, "exit") == 0 or strcmp(buffer, "EXIT") == 0) {
                Message m(MSG_TYPE_EXIT, "exit");
                write(sock_fd, reinterpret_cast<char *>(&m), sizeof(m));
                break;
            } else {
                Message m(MSG_TYPE_REQUEST, "");
                strcpy(m.message, buffer);
                write(sock_fd, reinterpret_cast<char *>(&m), sizeof(m));
            }
        }
    }
    return EXIT_SUCCESS;
}