#ifndef _UTILS_ARRAY_H_
#define _UTILS_ARRAY_H_

#include <memory>
#include <cstdint>

template<typename T>
struct Array {
    std::unique_ptr<T> buffer;
    size_t length;

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

    Array(const Array<T>& other) noexcept(std::is_nothrow_copy_constructible<T>::value)
      : buffer(nullptr),
        length(0) {
       this->buffer.reset(new (std::nothrow) T[other.length]);
       if (this->buffer == nullptr) {
         return;
       }
       this->length = other.length;
       memcpy(this->buffer.get(), other.buffer.get(), other.length);
    }

    Array<T>& operator=(Array<T>&& other) noexcept {
      this->length = other.length;
      this->buffer.swap(other.buffer);
      return *this;
    }
};

#endif //_UTILS_ARRAY_H_