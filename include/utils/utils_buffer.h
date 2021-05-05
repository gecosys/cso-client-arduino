#ifndef _UTILS_BUFFER_H_
#define _UTILS_BUFFER_H_

#include <memory>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <FreeRTOS.h>
#include <freertos/semphr.h>

#define NOT 0
#define POINTER_OBJECT 1
#define POINTER_ARRAY 2

template<typename ValueType, uint8_t IsPointer = NOT>
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
    //=======
    // COMMON
    //=======
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

    bool sync_write(uint32_t& idx) {
        // Timeout to get semaphore is 3s
        if (xSemaphoreTake(this->semaphore, 3000 / portTICK_PERIOD_MS) != pdTRUE) {
            return false;
        }
        if (this->size == this->capacity) {
            xSemaphoreGive(this->semaphore);
            return false;
        }

        idx = this->idx_write;
        this->idx_write = (this->idx_write + 1) % this->capacity;
        this->size++;
        xSemaphoreGive(this->semaphore);
        return true;
    }

    // This class just cares to raw pointer (not include smart pointer)
    // Use "std::is_pointer" will check raw pointer (detects smart pointer)
    template <uint8_t TIsPointer = IsPointer, 
                typename std::enable_if<TIsPointer != NOT && 
                    std::is_pointer<ValueType>::value>::type* = nullptr>
    void cleanup() {
        if (IsPointer == POINTER_OBJECT) {
            for (uint32_t idx = this->idx_read, count = 0; count < this->size; ++count) {
                delete reinterpret_cast<ValueType&>(this->buffer[idx]);
                idx = (idx + 1) % this->capacity;
            }
            return;
        }
        for (uint32_t idx = this->idx_read, count = 0; count < this->size; ++count) {
            delete[] reinterpret_cast<ValueType&>(this->buffer[idx]);
            idx = (idx + 1) % this->capacity;
        }
    }

    //========
    // DESTROY
    //========
    // "IsPointer" is pointer, this function is compiled
    template <uint8_t TIsPointer = IsPointer, 
                typename std::enable_if<TIsPointer != NOT && 
                    std::is_pointer<ValueType>::value>::type* = nullptr>
    void destroy() {
        cleanup();
        delete[] this->buffer;
    }

    // "IsPointer" is not pointer, this function is compiled
    template <uint8_t TIsPointer = IsPointer, 
                typename std::enable_if<TIsPointer == NOT && 
                    !std::is_pointer<ValueType>::value>::type* = nullptr>
    void destroy() {
        delete[] this->buffer;
    }

public:
    Buffer(uint32_t capacity) : size(0), capacity(capacity), idx_read(0), idx_write(0) {
        if (this->capacity == 0) {
            throw "Capacity has to larger than 0";
        }
        this->semaphore = xSemaphoreCreateMutex();
        this->buffer = new storage[this->capacity];
    }

    ~Buffer() noexcept {
        destroy();
    }

    //======
    // CLEAR
    //======
    template <uint8_t TIsPointer = IsPointer, 
                typename std::enable_if<TIsPointer == NOT && 
                    !std::is_pointer<ValueType>::value>::type* = nullptr>
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

    template <uint8_t TIsPointer = IsPointer, 
                typename std::enable_if<TIsPointer != NOT && 
                    std::is_pointer<ValueType>::value>::type* = nullptr>
    bool clear() {
        if (xSemaphoreTake(this->semaphore, 3000 / portTICK_PERIOD_MS) != pdTRUE) {
            return false;
        }
        cleanup();
        this->size = 0;
        this->idx_read = 0;
        this->idx_write = 0;
        xSemaphoreGive(this->semaphore);
        return true;
    }

    //======
    // PUSH
    //======
    template <uint8_t TIsPointer = IsPointer, 
                typename std::enable_if<TIsPointer == NOT && 
                    !std::is_pointer<ValueType>::value>::type* = nullptr>
    bool push(ValueType&& value) {
        uint32_t idx;
        if (!sync_write(idx)) {
            return false;
        }
        // "std::addressof" prevents user wrong implementation "operator &"
        // User can return address of some fields of object, not object's address
        new (static_cast<void*>(std::addressof(this->buffer[idx])))ValueType(value);
        return true;
    }

    template <uint8_t TIsPointer = IsPointer, 
                typename std::enable_if<TIsPointer == NOT && 
                    !std::is_pointer<ValueType>::value>::type* = nullptr>
    bool push(ValueType& value) {
        return push(std::move(value));
    }

    template <uint8_t TIsPointer = IsPointer, 
                typename std::enable_if<TIsPointer != NOT && 
                    std::is_pointer<ValueType>::value>::type* = nullptr>
    bool push(ValueType&& value) {
        uint32_t idx;
        if (!sync_write(idx)) {
            return false;
        }
        new (static_cast<void*>(std::addressof(this->buffer[idx])))ValueType(value);
        return true;
    }

    template <typename T,
                uint8_t TIsPointer = IsPointer, 
                    typename std::enable_if<TIsPointer != NOT && 
                        std::is_pointer<ValueType>::value>::type* = nullptr>
    bool push(ValueType& value, uint32_t len = 0) {
        uint32_t idx;
        if (!sync_write(idx)) {
            return false;
        }
        if (len != 0) {
            ValueType v = new T[len];
            memcpy(v, value, len * sizeof(T));
            new (static_cast<void*>(std::addressof(this->buffer[idx])))ValueType(v);
        }
        else new (static_cast<void*>(std::addressof(this->buffer[idx])))ValueType(value);
        return true;
    }

    //====
    // POP
    //====
    template <uint8_t TIsPointer = IsPointer, 
                typename std::enable_if<TIsPointer == NOT && 
                    !std::is_pointer<ValueType>::value>::type* = nullptr>
    bool pop(ValueType& ret) {
        uint32_t idx;
        if (!sync_read(idx)) {
            return false;
        }
        ret = reinterpret_cast<ValueType&>(this->buffer[idx]);
        return true;
    }

    template <typename T, 
                uint8_t TIsPointer = IsPointer, 
                    typename std::enable_if<TIsPointer != NOT && 
                        std::is_pointer<ValueType>::value>::type* = nullptr>
    bool pop(std::unique_ptr<T>& ret) {
        uint32_t idx;
        if (!sync_read(idx)) {
            return false;
        }
        ret.reset(reinterpret_cast<ValueType&>(this->buffer[idx]));
        return true;
    }
};

#endif //_UTILS_BUFFER_H_