#include "allocator_error.h"

AllocError::AllocError(AllocErrorType _type, std::string message)
    : runtime_error(message)
    , type(_type) {
}

AllocErrorType AllocError::getType() const 
{ 
    return type; 
}
