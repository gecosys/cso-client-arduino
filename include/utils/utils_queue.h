#ifndef _UTILS_QUEUE_H_
#define _UTILS_QUEUE_H_

#include <memory>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <FreeRTOS.h>
#include <freertos/semphr.h>
#include "utils_code.h"

// "Queue" is general class for any type.
// Except std::unique_ptr (because it doesn't has copy constructor)
// and raw pointer (can implement but not necessary yet)
template<typename ValueType>
class Queue {
private:
    // "std::aligned_storage" fix struct alignment
    using storage = typename std::aligned_storage<sizeof(ValueType), alignof(ValueType)>::type;
    SemaphoreHandle_t semaphore;
    storage* buffer;
    uint32_t size;
    uint32_t capacity;
    uint32_t idx_read;
    uint32_t idx_write;

private:
    Error::Code sync_read(uint32_t& idx) {
        // Timeout to get semaphore is 3s
        if (xSemaphoreTake(this->semaphore, 3000 / portTICK_PERIOD_MS) != pdTRUE) {
            return Error::CanNotSync;
        }
        if (this->size == 0) {
            xSemaphoreGive(this->semaphore);
            return Error::Empty;
        }

        idx = this->idx_read;
        this->idx_read = (this->idx_read + 1) % this->capacity;
        this->size--;
        xSemaphoreGive(this->semaphore);
        return Error::Nil;
    }

public:
    Queue(uint32_t capacity) : size(0), capacity(capacity), idx_read(0), idx_write(0) {
        assert(this->capacity != 0);
        this->semaphore = xSemaphoreCreateMutex();
        this->buffer = new storage[this->capacity];
    }

    ~Queue() noexcept {}

    Error::Code clear() {
        if (xSemaphoreTake(this->semaphore, 3000 / portTICK_PERIOD_MS) != pdTRUE) {
            return Error::CanNotSync;
        }
        this->size = 0;
        this->idx_read = 0;
        this->idx_write = 0;
        xSemaphoreGive(this->semaphore);
        return Error::Nil;
    }

    Error::Code push(ValueType&& value) {
        return push(value);
    }

    Error::Code push(ValueType& value) {
        // Timeout to get semaphore is 3s
        if (xSemaphoreTake(this->semaphore, 3000 / portTICK_PERIOD_MS) != pdTRUE) {
            return Error::CanNotSync;
        }
        if (this->size == this->capacity) {
            xSemaphoreGive(this->semaphore);
            return Error::Full;
        }

        uint32_t idx = this->idx_write;
        this->idx_write = (this->idx_write + 1) % this->capacity;
        this->size++;
        xSemaphoreGive(this->semaphore);
        // "std::addressof" prevents user wrong implementation "operator &"
        // User can return address of some fields of object, not object's address
        new (static_cast<void*>(std::addressof(this->buffer[idx])))ValueType(value);
        return Error::Nil;
    }

    // If "ValueType" is not std::share_ptr<T> =, this function will be complied
    template <typename T, 
              bool IsSharePtr = std::is_same<ValueType, std::unique_ptr<T>>::value, 
              typename std::enable_if<!IsSharePtr>::type* = nullptr>
    Error::Code pop(ValueType& ret) {
        uint32_t idx;
        Error::Code err = sync_read(idx);
        if (err != Error::Nil) {
            return err;
        }
        ret = reinterpret_cast<ValueType&>(this->buffer[idx]);
        return Error::Nil;
    }

    // If "ValueType" is std::share_ptr<T> =, this function will be complied
    // Must specify the argument "std::shared_ptr" to update the "nullptr" for 
    // the queue's word after "pop" to completely transfer ownership of the 
    // memory to the user.
    template <typename T, 
              bool IsSharePtr = std::is_same<ValueType, std::unique_ptr<T>>::value,
              typename std::enable_if<IsSharePtr>::type* = nullptr>
    Error::Code pop(ValueType& ret) {
        assert(ret.get() != nullptr);
        uint32_t idx;
        Error::Code err = sync_read(idx);
        if (err != Error::Nil) {
            return err;
        }
        ret.swap(reinterpret_cast<ValueType&>(this->buffer[idx]));
        return Error::Nil;
    }
};

#endif //_UTILS_QUEUE_H_