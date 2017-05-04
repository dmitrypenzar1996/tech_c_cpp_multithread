#include <stdio.h>
#include <memory>
#include <unordered_map>
#include <sys/signal.h>
#include <iostream>

#include "add.h"
#include "Socket.h"
#include "SocketException.h"
#include "WriteTask.h"


using std::cout;
using std::endl;
using std::make_shared;
using std::queue;
using std::unordered_map;


int main(int argNum, char** args)
{

	if (argNum != 2)
	{
		cout << "Usage: ./epoll_task.exe <char* portname>" << endl;
		exit(1);
	}

    char* portname = args[1];


	signal(SIGPIPE, SIG_IGN);
	
	int server_socket = create_server_socket(portname);
    
	// creating epoll data
	int epoll_listener = epoll_create1(0);
	epoll_event event;

	event.data.fd = server_socket; 
	event.events = EPOLLIN;
	if (epoll_ctl(epoll_listener,\
		   EPOLL_CTL_ADD, server_socket, &event) < 0)
	{
		fprintf(stderr, "Error %d, %s",\
			errno, strerror(errno));
		exit(errno);
	}

	epoll_event epoll_event_arr[MAX_EPOLL_EVENTS];

	
	unordered_map<int, Socket> clients_sockets;


	while(true)
	{
		int event_num = epoll_wait(epoll_listener, \
							epoll_event_arr,\
							MAX_EPOLL_EVENTS, -1);
							//-1 - block indefinitely

		if (event_num < 0)
		{
			fprintf(stderr, "Error %d, %s",\
				errno, strerror(errno));
			
		}
		for (int i = 0; i < event_num; ++i)
		{
			try
			{
			//std::cout << "Processing event " << i << endl;
			int sfd = epoll_event_arr[i].data.fd;
			if (sfd == server_socket)
			{
				cout << "Recieve new connection" << endl;
				sockaddr c_addr;
				socklen_t c_len = sizeof(c_addr);
				int new_connection = -1;
				// new connection
				while (true)
				{
					
					new_connection = accept(server_socket,\
											&c_addr,
											&c_len);
					if (new_connection < 0)
					{
						if ((errno == EAGAIN || errno == EWOULDBLOCK))
						{
						    // all incoming connections is proceded
							break;
						}
						else
						{
							fprintf(stderr, "Error %d, %s\n",\
										errno, strerror(errno));
							break;
							//continue;
						}
					}
					clients_sockets[new_connection] =Socket(new_connection, epoll_listener);
				}
			}
			else if ((epoll_event_arr[i].events & EPOLLOUT) || (epoll_event_arr[i].events & EPOLLIN))
			{
				if (epoll_event_arr[i].events & EPOLLOUT)
				{
					//socket is ready for write
					std::cout << "Writing to availiable socket" << endl;
					Socket& soc = clients_sockets[sfd];
					soc.write();
					std::cout << "Done" << endl;
				}
				if (epoll_event_arr[i].events & EPOLLIN)
				{
					std::cout << "Receive message" << endl;
					int is_closed = 0;
					while(true)
					{
						char buffer[MAX_MSG_LEN];
						ssize_t count = read(sfd, buffer, MAX_MSG_LEN);
						if (count < 0)
						{
							if (errno != EAGAIN)
							{
								fprintf(stderr, "Error %d, %s at socket %d",\
										errno, strerror(errno), sfd);
								is_closed = 1;
							}
							break;
						}
						else if (count == 0)
						{
							// connection is closed;
							is_closed = 1; 
							std::cout << "Close connection" << std::endl;
							break;
						}
						for (std::pair<const int, Socket>& cls : clients_sockets)
						{
							if (cls.first != sfd)
							{
								cls.second.add_writetask(buffer, count);
							}
						}
						std::cout << "Sent it for all clients" << endl;
					}
					if (is_closed)
					{
						clients_sockets.erase(sfd);
					}
					//std::cout << "Process message" << endl;
				}
		    }
			else
			{
					fprintf(stderr, "Error %d, %s at socket %d\n",\
								errno, strerror(errno), sfd);
					clients_sockets.erase(sfd);
			}
			}
			catch (SocketException& err)
			{

				cout << err.what() << endl;
				close(err.get_socket_fd());
				if (err.get_socket_fd())
				{
					exit(0);
				}
			}
		}
	}
    return 0;
}
