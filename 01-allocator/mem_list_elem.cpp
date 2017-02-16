#include "mem_list_elem.h"


MemListElem::Error::Error(std::string message) : message{message}
{}


MemListElem::MemListElem() : alloc_start{nullptr},
                             child(nullptr)
{}


void MemListElem::setAllocStart(char* _alloc_start)
{
    alloc_start = _alloc_start;
}


char* MemListElem::getAllocStart()
{
    return (char*) ((int64_t) alloc_start & ADDRESS_MASK) ;
}


size_t MemListElem::getSize()
{
    if (child)
    {
        return child->getAllocStart() - getAllocStart() ;
    }
    else // can't calculate size of block without child
    {
        throw Error("bad child");
    }
}


void MemListElem::setFreeFlag()
{
    alloc_start = (char*) (((int64_t) alloc_start) | FIRST_LEFT_BIT_MASK);
}


void MemListElem::unsetFreeFlag()
{
    alloc_start = (char*) (((int64_t) alloc_start) & ADDRESS_MASK);
}


char MemListElem::getFreeFlag()
{
    return char ((((size_t)alloc_start) & ( FIRST_LEFT_BIT_MASK)) >> 63) ;
}


void MemListElem::split2(MemListElem* _child, size_t chunk_size)
{
    _child->child = child;
    child  = _child;
    _child->setAllocStart(getAllocStart() + chunk_size);
}


MemListElem* MemListElem::getNext()
{
    if (getFreeFlag() && getAllocStart())
    {
        return *(MemListElem**) getAllocStart();
    }
    else // there is no memory for next if block is not free
    {
        throw Error("Element is not in free list ");
    }
}


void MemListElem::setNext(MemListElem* _next)
{
    if (getFreeFlag() && getAllocStart())
    {
        *(MemListElem**)getAllocStart() = _next;
    }
    else // there is no memory for next if block is not free
    {
        throw Error("Element is not free or has zero size");
    }
}


MemListElem* MemListElem::getChild()
{
    return child;
}


void MemListElem::setChild(MemListElem* _child)
{
    child = _child;
}
