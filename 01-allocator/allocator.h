#ifndef ALLOCATOR
#define ALLOCATOR
#include <string>
#include "allocator_pointer.h"
#include "allocator_error.h"
#include "mem_list_elem.h"
// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Pointer;
class Allocator;
class MemListElem;
/**
 * Wraps given memory area and provides defragmentation allocator interface on
 * the top of it.
 *
 *
 */




class Allocator {
public:
    Allocator(void* base, size_t size, double sys_frac = 0.01);
    Pointer alloc(size_t N);
    void realloc(Pointer& p, size_t N);
    void free(Pointer& p);
    void defrag();

private:
    char* mem_start;
    char* mem_end;
    double sys_frac;
    MemListElem* avail_head;
    MemListElem* avail_tail;
    MemListElem* free_head; // for unused MemListElems

    MemListElem* getFreeElem();
    MemListElem* getAvailElem(size_t chunk_size);
    void addNewFreeElemsChunk(MemListElem* start, MemListElem* end);
    void safeMemcpy(void* dst, void* src, size_t n);
    void addToAvailList(MemListElem* elem);
    void addToFreeList(MemListElem* elem);
    char moveSysWall();
    void joinWithFreeChildren(MemListElem* elem);
    char _realloc(Pointer& p, size_t);
    size_t align(size_t size);
};


#endif // ALLOCATOR
