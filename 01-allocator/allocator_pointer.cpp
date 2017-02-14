#include "allocator_pointer.h"

Pointer::Pointer(MemListElem* elem) : elem{elem}
{}

Pointer::Pointer() : elem{nullptr} {}

void* Pointer::get() const
{
    if (elem != nullptr) return elem->getAllocStart();
    return nullptr;
}


MemListElem* Pointer::getElem() const
{
    return elem;
}

void Pointer::setElem(MemListElem* elem_)
{
    elem = elem_;
}
