#ifndef _SYNCHRONIZATION_ARRAY_H_
#define _SYNCHRONIZATION_ARRAY_H_

#include <memory>
#include <cstdint>
#include <cstring>
#include "error/error_code.h"

template<typename T>
class Array {
public:
    std::unique_ptr<T> buffer;
    size_t length;

public:
    Array() noexcept
      : buffer(nullptr), 
        length(0) {}
          
    Array(T* buffer, size_t length) noexcept
      : buffer(buffer), 
        length(length) {}

    Array(Array<T>&& other) noexcept
      : buffer(nullptr),
        length(other.length) {
       this->buffer.swap(other.buffer);
    }

    Array<T>& operator=(const Array<T>& other) = delete;
    Array<T>& operator=(Array<T>&& other) noexcept {
      this->length = other.length;
      this->buffer.swap(other.buffer);
      return *this;
    }

    Error::Code copy(const Array<T>& other) noexcept {
      T* temp = new (std::nothrow) T[other.length];
      if (temp == nullptr) {
        return Error::NotEnoughMemory;
      }
      this->length = other.length;
      this->buffer.reset(temp);
      memcpy(this->buffer.get(), other.buffer.get(), other.length);
      return Error::Nil;
    }
};

#endif //_SYNCHRONIZATION_ARRAY_H_