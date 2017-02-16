#include "allocator.h"
const int64_t MemListElem::FIRST_LEFT_BIT_MASK = 0x8000000000000000L;
const int64_t MemListElem::ADDRESS_MASK = 0x7FFFFFFFFFFFFFFFL;

size_t Allocator::align(size_t size)
{
    size_t mod = size % sizeof(int64_t);
    if (mod == 0)
    {
        return size;
    }
    else
    {
        return size + (sizeof(int64_t) - mod);
    }
}


void Allocator::addToAvailList(MemListElem* elem)
{
    elem->setFreeFlag();
    elem->setNext(avail_head->getNext());
    avail_head->setNext(elem);
}

MemListElem* Allocator::getAvailElem(size_t chunk_size)
{
    if (chunk_size == 0)
    {
        return nullptr;
    }

    chunk_size = align(chunk_size);

    MemListElem* prev_elem = avail_head;
    MemListElem* cur_elem = avail_head->getNext();
    size_t elem_size = 0;

    while(cur_elem < avail_tail)
    {
        if (! cur_elem->getChild()) // see Allocator::joinWithFreeChildren
        {
            prev_elem->setNext(cur_elem->getNext());
            addToFreeList(cur_elem);
            cur_elem = prev_elem->getNext();
            continue;
        }

        elem_size = cur_elem->getSize();
        if (elem_size == chunk_size)
        {
            prev_elem->setNext(cur_elem->getNext());
            cur_elem->unsetFreeFlag();
            return cur_elem;
        }
        else if (elem_size > chunk_size)
        {
            MemListElem* new_elem = getFreeElem();
            cur_elem->split2(new_elem, chunk_size);

            new_elem->setFreeFlag();
            new_elem->setNext(cur_elem->getNext());

            prev_elem->setNext(new_elem);

            cur_elem->unsetFreeFlag();
            return cur_elem;
        }
        else
        {
            prev_elem = cur_elem;
            cur_elem = cur_elem->getNext();
        }
    }
    throw AllocError(AllocErrorType::NoMemory);
}

void Allocator::addToFreeList(MemListElem* elem)
{
    elem->unsetFreeFlag();
    elem->setAllocStart(nullptr);
    elem->setChild(free_head);
    free_head = elem;
}


MemListElem* Allocator::getFreeElem()
{
    MemListElem* child;
    while ((child = last_before_sys->getChild()) != avail_tail)
    {
        last_before_sys = child;
    }

    size_t available_size = last_before_sys->getSize();

    if (available_size - sizeof(void*) >= sizeof(MemListElem))
    {
        avail_tail->setAllocStart(avail_tail->getAllocStart() - sizeof(MemListElem));
        return new(avail_tail->getAllocStart()) MemListElem();
    }

    throw AllocError(AllocErrorType::NoMemory);
}


Allocator::Allocator(void* base, size_t size)
{
    size = size - (size % sizeof(int64_t));

    mem_start = (char*) align((size_t)base);
    mem_end   = mem_start + size;
    free_head = nullptr;

    avail_head = new(mem_end - sizeof(MemListElem)) MemListElem();
    avail_tail = new(mem_end - sizeof(MemListElem) * 2) MemListElem();

    avail_head->setAllocStart(mem_start);
    avail_head->setFreeFlag();
    avail_head->setNext(avail_tail);
    avail_head->setChild(avail_tail);


    avail_tail->setAllocStart(mem_end - sizeof(MemListElem) * 3);
    avail_tail->setFreeFlag();


    MemListElem* elem = new(avail_tail->getAllocStart()) MemListElem();

    elem->setAllocStart(mem_start + sizeof(MemListElem*)); // for head next

    elem->setChild(avail_tail);
    avail_head->setChild(elem);

    addToAvailList(elem);

    last_before_sys = elem;

}

Pointer Allocator::alloc(size_t chunk_size)
{
    chunk_size = align(chunk_size);

    MemListElem* elem = getAvailElem(chunk_size);

    return Pointer(elem);
}

void Allocator::joinWithFreeChildren(MemListElem* elem)
{

    MemListElem* child = elem->getChild();
    while ((child < avail_tail) && child->getFreeFlag())
    {
        MemListElem* new_child = child->getChild();
        child->setChild(nullptr);
        child = new_child;
        // after this operation avail list will be invalid,
        // we mark this fact by assigning to child field nullptr value. It's
        //valid because all valid elements in avail list have nonnull child
    }
    elem->setChild(child);

    if (child == avail_tail)
    {
        last_before_sys = elem;
    }
}

char Allocator::_realloc(Pointer& ptr, size_t chunk_size)
{

    MemListElem* elem = ptr.getElem();
    if (elem == nullptr)
    {
        ptr.setElem(getAvailElem(chunk_size));
        return 1;
    }

    size_t elem_size = elem->getSize();
    if (elem_size == chunk_size)
    {
        return 1;
    }
    else if (elem_size > chunk_size)
    {
        MemListElem* new_elem = getFreeElem();
        elem->split2(new_elem, chunk_size);

        addToAvailList(new_elem);
    }
    else
    {
        return 0;
    }
    return 1;
}


void Allocator::realloc(Pointer& ptr, size_t chunk_size)
{
    chunk_size = align(chunk_size);
    if (_realloc(ptr, chunk_size))
    {
        return;
    }
    else
    {
        MemListElem* elem = ptr.getElem();
        joinWithFreeChildren(ptr.getElem());
        if (_realloc(ptr, chunk_size))
        {
            return;
        }
        else
        {
            MemListElem* new_elem = getAvailElem(chunk_size);
            safeMemcpy(new_elem->getAllocStart(),
                       elem->getAllocStart(), elem->getSize());
            addToAvailList(elem);
            ptr.setElem(new_elem);
        }
    }
}

void Allocator::free(Pointer& ptr)
{
    MemListElem* ptrElem = ptr.getElem();
    if (ptrElem && (! ptrElem->getFreeFlag()))
    {
        ptr.setElem(nullptr);
        addToAvailList(ptrElem);
    }
    else
    {
        throw AllocError(AllocErrorType::InvalidFree);
    }
}

void Allocator::safeMemcpy(void* dst, void* src, size_t chunk_size)
{

    int64_t* cur_dst = (int64_t*)dst;
    int64_t* cur_src = (int64_t*)src;
    size_t ulong_size = chunk_size - (chunk_size % sizeof(int64_t));

    char* else_dst = (char*)(dst) + ulong_size;
    char* else_src = (char*)(src) + ulong_size;
    char* end_dst = (char*)(dst) + chunk_size;

    for(; cur_dst != (int64_t*)else_dst;) // copy by words
    {
        *(cur_dst++) = *(cur_src++);
    }

    for(; else_dst != end_dst;) // copy by bytes
    {
        *(else_dst++) = *(else_src++);
    }
}

void Allocator::defrag()
{
    avail_head->setNext(avail_tail); // free avail_list

    MemListElem* cur_elem = avail_head->getChild();
    MemListElem* prev_used_elem = avail_head;

    char* cur_mem_pos = cur_elem->getAllocStart();

    while(cur_elem < avail_tail)
    {
        if(! cur_elem->getFreeFlag())
        {
            size_t size = cur_elem->getSize();
            safeMemcpy(cur_mem_pos, cur_elem->getAllocStart(), size);
            cur_elem->setAllocStart(cur_mem_pos);
            cur_mem_pos += size;

            prev_used_elem->setChild(cur_elem);
            prev_used_elem = cur_elem;
        }
        cur_elem = cur_elem->getChild();
    }




    MemListElem* used_sys_border = (MemListElem*)avail_tail->getAllocStart();
    while((used_sys_border > (MemListElem*)avail_tail->getAllocStart()) &&\
                used_sys_border->getFreeFlag())
    {
        ++used_sys_border;
    }

    // we can safely remove all unused elements before the first used element
    avail_tail->setAllocStart((char*)used_sys_border - sizeof(MemListElem));

    for(MemListElem* elem = used_sys_border; elem < avail_tail; ++elem)
    {
        if (elem->getFreeFlag())
        {
            addToFreeList(elem);
        }

    }

    avail_tail->setChild(nullptr);

    MemListElem* elem = new(avail_tail->getAllocStart()) MemListElem();

    elem->setAllocStart(cur_mem_pos);
    elem->setChild(avail_tail);
    prev_used_elem->setChild(elem);

    addToAvailList(elem);

    last_before_sys = elem;
}

