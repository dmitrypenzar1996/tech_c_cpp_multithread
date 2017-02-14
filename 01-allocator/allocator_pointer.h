#ifndef ALLOCATOR_POINTER
#define ALLOCATOR_POINTER
#include "allocator.h"
#include "mem_list_elem.h"
// Forward declaration. Do not include real class definition
// to avoid expensive macros calculations and increase compile speed
class Allocator;
class MemListElem;

class Pointer {
private:
    MemListElem* elem;
public:
    Pointer();
    Pointer(MemListElem* elem);
    void* get() const;
    MemListElem* getElem() const;
    void setElem(MemListElem* elem);
};

#endif //ALLOCATOR_POINTER
