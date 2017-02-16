//
// Created by penzardmitry on 14.02.17.
//

#ifndef TECHNOSPHERE_C_MULTITHREAD_MEM_LIST_ELEM_H
#define TECHNOSPHERE_C_MULTITHREAD_MEM_LIST_ELEM_H
#include "allocator.h"

class MemListElem
{
public:
    /*
     * return new Element with all fields set to nullptr
     */
    MemListElem();


    /*
     * return start of block coresponding to this element
     */
    char* getAllocStart();


    /*
     * set void to start of allocated block
     * @params _alloc_start start of allocated block
     */
    void setAllocStart(char* _alloc_start);


    /*
     * return size of block or if child == nullptr throw BadChild error
     */
    size_t getSize();


    /*
     * marks block as free
     */
    void setFreeFlag();


    /*
     * marks block is used
     */
    void unsetFreeFlag();


    /*
     * return is block free or not
     */
    char getFreeFlag();


    /*
     * save pointer to next free element in list
     * elem must have size >= 8 bytes (for storing pointer) and be free
     * throw Error if requirements are not satisfied
     */
    void setNext(MemListElem* _next);


    /*
     * return next element in free list 
     * elem should be free and have size >=8 bytes
     */
    MemListElem* getNext();


    /*
     * set child of element
     * @params _child elem, refers to block after block our elem refering to
     */
    void setChild(MemListElem* _child);


    /*
     * return child of element
     */
    MemListElem* getChild();


    /*
     * split block elem refers to into two,
     * first of chunk_size, second : size - chunk_size
     * set elem ref to first block, child ref - to second one
     * set elem child ref to child, child ref - to previous child of elem
     * @params chunk_size size of first block
     */
    void split2(MemListElem* child,\
            size_t chunk_size);
private:
    /*
     * error struct for MemListElem
     */
    struct Error
    {
        std::string message;
        Error(std::string message = "");
    };


    char* alloc_start; // pointer to start of block


    MemListElem* child;// refers to block after block our elem refering to


    /*
     * mask for retrieving of free flag  
     */
    static const int64_t FIRST_LEFT_BIT_MASK;


    /*
     * mask for retriving address of coresponding block
     */
    static const int64_t ADDRESS_MASK;
};


#endif //TECHNOSPHERE_C_MULTITHREAD_MEM_LIST_ELEM_H
