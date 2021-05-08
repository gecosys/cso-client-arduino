#ifndef _MESSAGE_ARRAY_H_
#define _MESSAGE_ARRAY_H_

#include <memory>
#include <cstdint>

template<typename T>
struct Array {
    std::shared_ptr<T> ptr;
    uint16_t length;

    Array() : ptr(nullptr), length(0) {}
};

#endif //_MESSAGE_ARRAY_H_