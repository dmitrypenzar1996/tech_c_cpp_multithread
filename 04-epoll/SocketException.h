#ifndef __SOCKET_EXCEPTION__
#define __SOCKET_EXCEPTION__
#include <errno.h>
#include <event.h>
#include <exception>
#include <fcntl.h>
#include <memory>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

class SocketException : std::exception {
private:
    static const size_t MAX_MSG_SIZE;
    int errcode;
    int socket_fd;
    char* msg;

public:
    SocketException();
    SocketException(int _errcode, int socket_fd = -1);
    SocketException(const char* err_str, int socket_fd = -1);
    ~SocketException();
    SocketException(SocketException&) = delete;
    SocketException& operator=(SocketException&) = delete;
    SocketException(SocketException&&);
    SocketException& operator=(SocketException&&);
    virtual const char* what() const throw();
    int get_socket_fd() { return socket_fd; }
    friend void swap(SocketException& first, SocketException& second);
};
#endif
