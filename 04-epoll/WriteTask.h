#ifndef __WRITE_TASK__
#define __WRITE_TASK__
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

struct WriteTask {
    ssize_t size;
    ssize_t offset;
    char* buffer;
    WriteTask();
    WriteTask(const char* buffer, ssize_t size, ssize_t offset = 0);
    WriteTask(WriteTask&& tmp);
    WriteTask& operator=(WriteTask&& tmp);
    WriteTask(const WriteTask&) = delete;
    WriteTask& operator=(const WriteTask&) = delete;
    ~WriteTask();
    friend void swap(WriteTask& first, WriteTask& second);
};
#endif
