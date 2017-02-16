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

/*
 * Wraps given memory area and provides defragmentation allocator interface on
 * the top of it.
 *
 * To allow defragmentation Allocator store links to allocated blocks of memory
 * and return links to this links. This links is stored in sys part of memory.
 * We add them from end part of given memory.
 * pointers to allocated but now free blocks are stored in avail_list,
 * avail_head - is head of this list, it always has size 8 bytes to store pointer
 * to next element
 * avail_tail - is tail of this list, it's alloc_start refers to start of 
 * sys part
 * last_before_sys - is element before avail_tail
 * pointers which are not using now are stored in free_list
 * free_head - head of this list
 * 
 *So, it's looks like something like this
 *                Our allocator memory:
 *Data|10   |xxxxx|7    |xxxxx|xxxxx|12   |6 12|5  7|4  8|3  9|2 10|7   0|1  11|
 *    |next |some |free |some |some |free |elem|elem|elem|elem|elem|avail|avail|
 *    |     |stuff|block|stuff|stuff|block|    |    |    |    |    |tail | head|
 *    1     2     3     4     5     6     7    8    9    10   11   12   13     14
 *Var mem                           last                                       mem
 *    start                         before                                     end
 *                                  sys
 *
 * where
 *
 * |x  y|
 * |elem|  x = alloc_start
 *         y = child
 * 
 * |z    | z = next
 * |free |
 * |block|
 */

class Allocator {
public:
    /*
     * creates an new allocator
     * @param base start of allocator memory
     * @param size size of this memory
     * @return new allocator
     */
    Allocator(void* base, size_t size);
    Allocator(const Allocator&) = delete;
    Allocator(Allocator&&) = default;
    Allocator& operator=(const Allocator&) = delete;
    Allocator& operator=(Allocator&&) = default;
    ~Allocator() = default;
    /*
     * try to allocate memory of given size
     * if its possible returns pointer
     * otherwise throw AllocError
     *
     * @param N size of memory to allocate
     * return pointer to allocated memory or n
    */
    Pointer alloc(size_t N);

    /*
     * try to change size of allocated chunk. If it's possible
     * just resizes the size of block, otherwise try to alloc block of
     * larger size and copy content of previous block to newly allocated.
     * If there is no enough memory throw AllocError
     * If pointer refers to null alloc is called.
     * 
     * @params p Pointer to be allocated
     * @params N new memory size
     * return pointer with realloced memory or throw AllocError
     */
    void realloc(Pointer& p, size_t N);


    /*
     * Frees memory given to p and set p ref to memory to nullptr.
     * If p has already freed throw AllocError
     */
    void free(Pointer& p);

    /*
     * Defragments memory. All allocated blocks are moved to the begin of memory.
     * Also try to delete unused sys memory.
     */
    void defrag();

private:
    char* mem_start;
    char* mem_end;
    MemListElem* last_before_sys; // last block before sys part
    MemListElem* avail_head; // head of list of free blocks
    MemListElem* avail_tail; // tail of list of free blocks
    MemListElem* free_head; // head of unused elems

    /*
     * return unused elem 
     */
    MemListElem* getFreeElem();

    /*
     * @params chunk_size size of memory to allocate
     * return elem refering to block of requested size of throw AllocError 
     */ 
    MemListElem* getAvailElem(size_t chunk_size);

    /*
     * Platform-independent way to copy from to possibly crossing dst and src
     * (only for Allocator)
     * @params dst pointer to destination memory begin
     * @params src pointer to source memeory begin
     * @params n bytes num to be copied
     */
    void safeMemcpy(void* dst, void* src, size_t n);

    /*
     * add elem to head of avail list
     * @params elem - free elem
     */
    void addToAvailList(MemListElem* elem);

    /*
     * add elem to head of free list
     * @params elem unused elem
     */
    void addToFreeList(MemListElem* elem);

    /*
     * merge blocks of elem and it's next free childs (until system block or
     * first used child) 
     * @params elem to start merge
     */
    void joinWithFreeChildren(MemListElem* elem);


    /*
     * private version of realloc. Don't call joinWithFreeChildren 
     * @p pointer to be reallocated
     * n new size of memory
     */
    char _realloc(Pointer& p, size_t n);

    /*
     * floor(size / size of int64_t)
     * @params size size of requested block
     */
    size_t align(size_t size);
};


#endif // ALLOCATOR
