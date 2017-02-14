//
// Created by penzardmitry on 14.02.17.
//

#ifndef TECHNOSPHERE_C_MULTITHREAD_MEM_LIST_ELEM_H
#define TECHNOSPHERE_C_MULTITHREAD_MEM_LIST_ELEM_H
#include "allocator.h"

class MemListElem
{
public:
    MemListElem();
    char* getAllocStart();
    void setAllocStart(char* _alloc_start);
    size_t getSize();
    void setFreeFlag();
    void unsetFreeFlag();
    char getFreeFlag();
    void setNext(MemListElem* _next);
    MemListElem* getNext();
    void setChild(MemListElem* _child);
    MemListElem* getChild();
    void split2(MemListElem* child,\
            size_t chunk_size);
private:
    struct Error
    {
        std::string message;
        Error(std::string message = "");
    };
    char* alloc_start;
    MemListElem* child;
    static const int64_t FIRST_LEFT_BIT_MASK;
    static const int64_t ADDRESS_MASK;
};


#endif //TECHNOSPHERE_C_MULTITHREAD_MEM_LIST_ELEM_H
