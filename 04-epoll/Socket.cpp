#include "Socket.h"

void swap(Socket& first, Socket& second) {
    using std::swap;
    swap(first.epoll_listener, second.epoll_listener);
    swap(first.socket_fd, second.socket_fd);
    swap(first.write_queue, second.write_queue);
}

Socket::Socket()
    : epoll_listener(-1)
    , socket_fd(-1) {}

Socket::Socket(int _socket_fd, int _epoll_listener)
    : socket_fd(_socket_fd)
    , epoll_listener(_epoll_listener) {
    set_notblocking(socket_fd);
    epoll_event event;
    event.data.fd = socket_fd;
    event.events = EPOLLIN;
    if (epoll_ctl(epoll_listener, EPOLL_CTL_ADD, socket_fd, &event)) {
        throw SocketException(errno, socket_fd);
    }
}

Socket& Socket::operator=(Socket&& tmp) {
    swap(*this, tmp);
    return *this;
}

Socket::Socket(Socket&& tmp)
    : Socket() { swap(*this, tmp); }

bool Socket::has_no_task() { return write_queue.empty(); }

void Socket::write() {
    WriteTask& task = write_queue.front();
    ssize_t remain = task.size - task.offset;
    ssize_t write_num = ::write(socket_fd, task.buffer + task.offset, remain);
    if (write_num < 0) {
        throw SocketException(errno, socket_fd);
    }
    task.offset += write_num;
    if (task.offset == task.size) {
        write_queue.pop();
    }
    if (write_queue
            .empty()) // if all is writen, remove from waiting for write events
    {
        epoll_event event;
        event.data.fd = socket_fd;
        event.events = EPOLLIN;
        if (epoll_ctl(epoll_listener, EPOLL_CTL_MOD, socket_fd, &event)) {
            throw SocketException(errno, socket_fd);
        }
    }
}

void Socket::add_writetask(char* buffer, ssize_t size) {
    if (socket_fd == -1) {
        throw SocketException("Socket is moved", socket_fd);
    }

    if (write_queue.empty()) {
        // set event for both in and writing
        epoll_event event;
        event.data.fd = socket_fd;
        event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
        if (epoll_ctl(epoll_listener, EPOLL_CTL_MOD, socket_fd, &event) < 0) {
            throw SocketException(errno, socket_fd);
        }
    }

    write_queue.push(WriteTask(buffer, size));
}
