#ifndef _UTILS_BUFFER_H_
#define _UTILS_BUFFER_H_

#include <memory>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <FreeRTOS.h>
#include <freertos/semphr.h>

// "Buffer" is general class for any type.
// Except std::unique_ptr (because it doesn't has copy constructor)
// and raw pointer (can implement but not necessary yet)
template<typename ValueType>
class Buffer {
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
    bool sync_read(uint32_t& idx) {
        // Timeout to get semaphore is 3s
        if (xSemaphoreTake(this->semaphore, 3000 / portTICK_PERIOD_MS) != pdTRUE) {
            return false;
        }
        if (this->size == 0) {
            xSemaphoreGive(this->semaphore);
            return false;
        }

        idx = this->idx_read;
        this->idx_read = (this->idx_read + 1) % this->capacity;
        this->size--;
        xSemaphoreGive(this->semaphore);
        return true;
    }

public:
    Buffer(uint32_t capacity) : size(0), capacity(capacity), idx_read(0), idx_write(0) {
        assert(this->capacity != 0);
        this->semaphore = xSemaphoreCreateMutex();
        this->buffer = new storage[this->capacity];
    }

    ~Buffer() noexcept {}

    bool clear() {
        if (xSemaphoreTake(this->semaphore, 3000 / portTICK_PERIOD_MS) != pdTRUE) {
            return false;
        }
        this->size = 0;
        this->idx_read = 0;
        this->idx_write = 0;
        xSemaphoreGive(this->semaphore);
        return true;
    }

    bool push(ValueType&& value) {
        // Timeout to get semaphore is 3s
        if (xSemaphoreTake(this->semaphore, 3000 / portTICK_PERIOD_MS) != pdTRUE) {
            return false;
        }
        if (this->size == this->capacity) {
            xSemaphoreGive(this->semaphore);
            return false;
        }

        uint32_t idx = this->idx_write;
        this->idx_write = (this->idx_write + 1) % this->capacity;
        this->size++;
        xSemaphoreGive(this->semaphore);
        // "std::addressof" prevents user wrong implementation "operator &"
        // User can return address of some fields of object, not object's address
        new (static_cast<void*>(std::addressof(this->buffer[idx])))ValueType(value);
        return true;
    }

    bool push(ValueType& value) {
        return push(std::move(value));
    }

    // If "ValueType" is not std::share_ptr<T> =, this function will be complied
    template <typename T, 
              bool IsSharePtr = std::is_same<ValueType, std::unique_ptr<T>>::value, 
              typename std::enable_if<!IsSharePtr>::type* = nullptr>
    bool pop(ValueType& ret) {
        uint32_t idx;
        if (!sync_read(idx)) {
            return false;
        }
        ret = reinterpret_cast<ValueType&>(this->buffer[idx]);
        return true;
    }

    // If "ValueType" is std::share_ptr<T> =, this function will be complied
    // Must specify the argument "std::shared_ptr" to update the "nullptr" for 
    // the buffer's word after "pop" to completely transfer ownership of the 
    // memory to the user.
    template <typename T, 
              bool IsSharePtr = std::is_same<ValueType, std::unique_ptr<T>>::value,
              typename std::enable_if<IsSharePtr>::type* = nullptr>
    bool pop(ValueType& ret) {
        assert(ret.get() != nullptr);
        uint32_t idx;
        if (!sync_read(idx)) {
            return false;
        }
        ret.swap(reinterpret_cast<ValueType&>(this->buffer[idx]));
        return true;
    }
};

#endif //_UTILS_BUFFER_H_