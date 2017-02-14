#ifndef ALLOCATOR_ERROR
#define ALLOCATOR_ERROR

#include <stdexcept>
#include <string>

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError : std::runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message = "");

    AllocErrorType getType() const;
};

#endif //ALLOCATOR_ERROR
