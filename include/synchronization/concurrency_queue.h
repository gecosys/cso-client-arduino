#ifndef _SYNCHRONIZATION_CONCURRENCY_QUEUE_H_
#define _SYNCHRONIZATION_CONCURRENCY_QUEUE_H_

#include <atomic>
#include <memory>
#include <cstdint>
#include <cstring>
#include <utility>
#include <type_traits>
#include "spin_lock.h"
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
            throw std::runtime_error("[utils_concurrency_queue/ConcurrencyQueue(...)]Capacity has to be larger than 0");
        }
        this->buffer = new (std::nothrow) storage[this->capacity];
        if (this->buffer == nullptr) {
            throw std::runtime_error("[utils_concurrency_queue/ConcurrencyQueue(...)]Not enough memory to create array");
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
    template <typename std::enable_if<std::is_nothrow_move_constructible<ValueType>::value>::type* = nullptr>
    Error::Code push(ValueType&& value) {
        uint32_t index;
        if (!syncPush(index)) {
            return Error::Utils_ConcurrencyQueue_Full;
        }
        // "std::addressof" prevents user wrong implementation "operator &"
        // User can return address of some fields of object, not object's address
        new (static_cast<void*>(std::addressof(this->buffer[index])))ValueType(std::move(value));
        return Error::Nil;
    }

    // This function will be called if "ValueType" has copy constructor
    template <typename std::enable_if<std::is_nothrow_copy_constructible<ValueType>::value>::type* = nullptr>
    Error::Code push(ValueType& value) {
        uint32_t index;
        if (!syncPush(index)) {
            return Error::Utils_ConcurrencyQueue_Full;
        }
        // "std::addressof" prevents user wrong implementation "operator &"
        // User can return address of some fields of object, not object's address
        new (static_cast<void*>(std::addressof(this->buffer[index])))ValueType(value);
        return Error::Nil;
    }

    std::pair<Error::Code, ValueType> pop() {
        std::pair<Error::Code, ValueType> ret(Error::Nil, ValueType());
        // Check empty
        if (this->size.load() == 0) {
            ret.first = Error::Utils_ConcurrencyQueue_Empty;
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
        std::swap(ret.second, *ptr);
        return ret;
    }
};

#endif //_SYNCHRONIZATION_CONCURRENCY_QUEUE_H_