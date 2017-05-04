#include "WriteTask.h"

WriteTask::WriteTask(): size(0), offset(0), buffer(nullptr){}

void swap(WriteTask& first, WriteTask& second)
{
	using std::swap;
	swap(first.size, second.size);
	swap(first.offset, second.offset);
	swap(first.buffer, second.buffer);
}


WriteTask::WriteTask(const char* _buffer, ssize_t _size, ssize_t _offset) : size(_size), offset(_offset)
{
	buffer = new char[size];
	strncpy(buffer, _buffer, size);
}

WriteTask::WriteTask(WriteTask&& tmp): WriteTask()
{
	swap(*this, tmp);
}

WriteTask& WriteTask::operator=(WriteTask&& tmp)
{
	swap(*this, tmp);
	return *this;
}

WriteTask::~WriteTask()
{
	delete[] buffer;
}
