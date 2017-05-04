#ifndef __ADD__SOCKET__
#define __ADD__SOCKET__
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

#include "SocketException.h"

#define MAX_MSG_LEN 256
#define MAX_EPOLL_EVENTS 128
#define MAX_CONNECTIONS_NUM 128
void set_notblocking(int sock_fd);
int create_server_socket(const char* port_name);
#endif
