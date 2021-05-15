#ifndef _UTILS_CONCURRENCY_QUEUE_H_
#define _UTILS_CONCURRENCY_QUEUE_H_

#include <atomic>
#include <memory>
#include <cstdint>
#include <cstring>
#include <utility>
#include <type_traits>
#include "utils_code.h"
#include "utils_safe.h"
#include "utils_spin_lock.h"

// "ConcurrencyQueue" is general class for any type.
// Except std::unique_ptr (because it doesn't has copy constructor)
// and raw pointer (can implement but not necessary yet)
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

public:
    ConcurrencyQueue(uint32_t capacity) 
        : spin(), 
          size(0), 
          capacity(capacity), 
          index_read(0), 
          index_write(0) {
        if (this->capacity <= 0) {
            throw std::runtime_error("[utils_concurrency_queue/ConcurrencyQueue(...)]Capacity has to be larger than 0");
        }
        this->buffer = Safe::new_arr<storage>(this->capacity);
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

    Error::Code push(ValueType&& value) {
        return push(value);
    }

    Error::Code push(ValueType& value) {
        if (this->size.fetch_add(1) >= this->capacity) {
            this->size.fetch_sub(1);
            return Error::Full;
        }

        // Use "SpinLock" to guard in case multiple threads call this function together
        this->spin.lock();
        uint32_t index = this->index_write;
        this->index_write = (this->index_write + 1) % this->capacity;
        this->spin.unlock();
        // "std::addressof" prevents user wrong implementation "operator &"
        // User can return address of some fields of object, not object's address
        new (static_cast<void*>(std::addressof(this->buffer[index])))ValueType(value);
        return Error::Nil;
    }

    std::pair<Error::Code, ValueType> pop() {
        std::pair<Error::Code, ValueType> ret(Error::Nil, ValueType());
        // Check empty
        if (this->size.fetch_sub(1) <= 0) {
            this->size.fetch_add(1);
            ret.first = Error::Empty;
            return ret;
        }

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

#endif //_UTILS_CONCURRENCY_QUEUE_H_