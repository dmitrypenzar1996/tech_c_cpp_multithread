#ifndef __SOCKET__
#define __SOCKET__
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <event.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include "add.h"
#include "WriteTask.h"
#include "SocketException.h"
using std::queue;

class Socket
{
private:
    int socket_fd;
	int epoll_listener;
    queue<WriteTask> write_queue;
public:
	Socket();
	Socket(int _socket_fd, int _epoll_listener);
    Socket(const Socket& other)=delete;
    Socket& operator=(const Socket& other)=delete;
    Socket(Socket&& tmp); 
    Socket& operator=(Socket&& tmp);
    ~Socket() {close(socket_fd);}
    void add_writetask(char* buffer, ssize_t size);
    void write();
    bool has_no_task();
	friend void swap(Socket& first, Socket& second);
};
#endif
