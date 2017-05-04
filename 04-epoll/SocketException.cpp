#include "SocketException.h"

const size_t SocketException::MAX_MSG_SIZE = 128; 

SocketException::SocketException() : errcode(-1), socket_fd(-1), msg(nullptr) {}

void swap(SocketException& first, SocketException& second)
{
	using std::swap;
	swap(first.errcode, second.errcode);
	swap(first.socket_fd, second.socket_fd);
	swap(first.msg, second.msg);
}

SocketException::SocketException(SocketException&& temp): SocketException()
{
	swap(*this, temp);
}

SocketException& SocketException::operator=(SocketException&& temp)
{
	swap(*this, temp);
	return *this;
}



SocketException::SocketException(int _errcode, int socket_fd)
{
    errcode = _errcode;
	socket_fd = socket_fd;
    msg = new char[MAX_MSG_SIZE];
    snprintf(msg, MAX_MSG_SIZE, "SocketError, error code %d : %s", errcode, strerror(errcode));
}

SocketException::SocketException(const char* err_str, int socket_fd)
{
    msg = new char[strlen(err_str)];
    strcpy(msg, err_str);
    errcode = -1;
}

SocketException::~SocketException()
{
    delete[] msg;
}

const char* SocketException::what() const throw()
{
    return msg;   
}

