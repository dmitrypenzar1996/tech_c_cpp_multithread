C=gcc
CXX=g++
CCFLAGS=
CXXFLAGS=--std=c++11
all: epoll

clean:
	rm -f epoll_task.exe

epoll:
	$(CXX) $(CXXFLAGS) epoll_task.cpp Socket.cpp SocketException.cpp WriteTask.cpp add.cpp -o epoll_task.exe
