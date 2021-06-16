#ifndef _SYNCHRONIZATION_CONCURRENCY_QUEUE_H_
#define _SYNCHRONIZATION_CONCURRENCY_QUEUE_H_

#include <atomic>
#include <memory>
#include <cstdint>
#include <type_traits>
#include "spin_lock.h"
#include "utils/result.h"
#include "error/error_code.h"

// "ConcurrencyQueue" is general class for any type.
// Except raw pointer (can implement but not necessary yet)
template<typename ValueType>
class ConcurrencyQueue {
private:
    // "std::aligned_storage" fix struct alignment
    using storage = typename std::aligned_storage<sizeof(ValueType), alignof(ValueType)>::type;
    // Not necessarily using smart pointer, using raw pointer is simpler
    storage* buffer;
    SpinLock spin;
    // Using "atomic" for "size" will be faster than putting "size" into "spin_lock" block
    std::atomic<uint32_t> size;
    uint32_t capacity;
    uint32_t index_read;
    uint32_t index_write;

private:
    bool syncPush(uint32_t& index) {
        if (this->size.fetch_add(1) >= this->capacity) {
            this->size.fetch_sub(1);
            return false;
        }

        // Use "SpinLock" to guard in case multiple threads call this function together
        this->spin.lock();
        index = this->index_write;
        this->index_write = (this->index_write + 1) % this->capacity;
        this->spin.unlock();
        return true;
    }

public:
    ConcurrencyQueue() = delete;
    ConcurrencyQueue(ConcurrencyQueue&& other) = delete;
    ConcurrencyQueue(const ConcurrencyQueue& other) = delete;
    ConcurrencyQueue& operator=(const ConcurrencyQueue& other) = delete;

    ConcurrencyQueue(uint32_t capacity) 
        : spin(), 
          size(0), 
          capacity(capacity), 
          index_read(0), 
          index_write(0) {
        if (this->capacity <= 0) {
            throw "[syncchronization/ConcurrencyQueue(uint32_t capacity)]Capacity has to be larger than 0";
        }
        this->buffer = new (std::nothrow) storage[this->capacity];
        if (this->buffer == nullptr) {
            throw "[syncchronization/ConcurrencyQueue(uint32_t capacity)]Not enough memory to create array";
        }
    }

    ~ConcurrencyQueue() noexcept {
        delete[] this->buffer;
    }

    void clear() {
        // "fetch_add" or "fetch_sub" uses "std::memory_order_seq_cst"
        this->size.store(0, std::memory_order_seq_cst);
        this->spin.lock();
        this->index_read = 0;
        this->index_write = 0;
        this->spin.unlock();
    }

    // This function will be called if "ValueType" has move constructor
    template <typename T = ValueType,
              typename std::enable_if<std::is_nothrow_move_constructible<T>::value>::type* = nullptr>
    Error::Code push(T&& value) {
        uint32_t index;
        if (!syncPush(index)) {
            return Error::Synchronization_ConcurrencyQueue_Full;
        }
        // "std::addressof" prevents user wrong implementation "operator &"
        // User can return address of some fields of object, not object's address
        new (static_cast<void*>(std::addressof(this->buffer[index])))T(std::move(value));
        return Error::Nil;
    }

    // This function will be called if "ValueType" has copy constructor
    template <typename T = ValueType, 
              typename std::enable_if<std::is_nothrow_copy_constructible<T>::value>::type* = nullptr>
    Error::Code push(T& value) {
        uint32_t index;
        if (!syncPush(index)) {
            return Error::Synchronization_ConcurrencyQueue_Full;
        }
        // "std::addressof" prevents user wrong implementation "operator &"
        // User can return address of some fields of object, not object's address
        new (static_cast<void*>(std::addressof(this->buffer[index])))T(value);
        return Error::Nil;
    }

    Result<ValueType> pop() {
        Result<ValueType> ret(Error::Nil, ValueType());
        // Check empty
        if (this->size.load() == 0) {
            ret.errorCode = Error::Synchronization_ConcurrencyQueue_Empty;
            return ret;
        }
        this->size.fetch_sub(1);

        // Use "SpinLock" to guard in case multiple threads call this function together
        this->spin.lock();
        uint32_t index = this->index_read;
        this->index_read = (this->index_read + 1) % this->capacity;
        this->spin.unlock();

        // Get value
        ValueType* ptr = reinterpret_cast<ValueType*>(std::addressof(this->buffer[index]));
        // Use "std::swap" completely transfer ownership of the memory to the user
        // if "ValueType" is "std::share_ptr" or struct containing "std :: share_ptr" fields
        std::swap(ret.data, *ptr);
        return ret;
    }
};

#endif //_SYNCHRONIZATION_CONCURRENCY_QUEUE_H_