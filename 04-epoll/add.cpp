#include "add.h"

int create_server_socket(const char* port_name) {
    char* flag;
    uint32_t port = strtol(port_name, &flag, 10);
    if (!flag) // port name is not valid
    {
        throw SocketException("Bad port name");
    }

    sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd < 0) {
        throw SocketException(errno);
    }

    if (bind(server_fd, (sockaddr*)&sock_addr, sizeof(sock_addr)) < 0) {
        throw SocketException(errno, server_fd);
    }

    set_notblocking(server_fd);

    // set listening for port
    if (listen(server_fd, MAX_CONNECTIONS_NUM) < 0) {
        throw SocketException("Can't listen port");
    }

    return server_fd;
}

void set_notblocking(int sock_fd) {
    int opts = fcntl(sock_fd, F_GETFL, 0);
    if (opts < 0) {
        throw SocketException(errno, sock_fd);
    }
    opts |= O_NONBLOCK;
    if (fcntl(sock_fd, F_SETFL, opts) < 0) {
        throw SocketException(errno, sock_fd);
    }
}
