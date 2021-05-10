#ifndef _UTILS_QUEUE_H_
#define _UTILS_QUEUE_H_

#include <atomic>
#include <memory>
#include <cstdint>
#include <cstring>
#include <utility>
#include <type_traits>
#include "utils_code.h"
#include "utils_safe.h"
#include "utils_spin_lock.h"

// "Queue" is general class for any type.
// Except std::unique_ptr (because it doesn't has copy constructor)
// and raw pointer (can implement but not necessary yet)
template<typename ValueType>
class Queue {
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
    #define SYNC_READ(index, ret)                                   \
    if (this->size.fetch_sub(1) <= 0) {                             \
        this->size.fetch_add(1);                                    \
        ret.first = Error::Empty;                                   \
        return ret;                                                 \
    }                                                               \
    this->spin.lock();                                              \
    index = this->index_read;                                       \
    this->index_read = (this->index_read + 1) % this->capacity;     \
    this->spin.unlock();

    #define SYNC_WRITE(index)                                       \
    if (this->size.fetch_add(1) >= this->capacity) {                \
        this->size.fetch_sub(1);                                    \
        return Error::Full;                                         \
    }                                                               \
    this->spin.lock();                                              \
    index = this->index_write;                                      \
    this->index_write = (this->index_write + 1) % this->capacity;   \
    this->spin.unlock();

public:
    Queue(uint32_t capacity) 
        : spin(), 
          size(0), 
          capacity(capacity), 
          index_read(0), 
          index_write(0) {
        assert(this->capacity > 0);
        this->buffer = Safe::new_arr<storage>(this->capacity);
        if (this->buffer == nullptr) {
            throw "[utils_queue/Queue(uin32_t capacity)]Not enough mem";
        }
    }

    ~Queue() noexcept {
        delete[] this->buffer;
    }

    void clear() {
        // "fetch_add" or "fetch_sub" uses "std::memory_order_seq_cst"
        this->size.store(0, std::memory_order_seq_cst);
        LOCK(&this->spinLock);
        this->index_read = 0;
        this->index_write = 0;
        UNLOCK(&this->spinLock);
    }

    Error::Code push(ValueType&& value) {
        return push(value);
    }

    Error::Code push(ValueType& value) {
        uint32_t index;
        SYNC_WRITE(index)
        // "std::addressof" prevents user wrong implementation "operator &"
        // User can return address of some fields of object, not object's address
        new (static_cast<void*>(std::addressof(this->buffer[index])))ValueType(value);
        return Error::Nil;
    }

    // If "ValueType" is not std::share_ptr<T> =, this function will be complied
    template <typename T, 
              bool IsSharePtr = std::is_same<ValueType, std::unique_ptr<T>>::value, 
              typename std::enable_if<!IsSharePtr>::type* = nullptr>
    std::pair<Error::Code, ValueType> pop() {
        uint32_t index;
        std::pair<Error::Code, ValueType> ret;
        SYNC_READ(index, ret)
        ValueType tmp = reinterpret_cast<ValueType&>(this->buffer[index]);
        ret.second = tmp;
        tmp.~ValueType();
        return ret;
    }

    // If "ValueType" is std::share_ptr<T> =, this function will be complied
    // Must specify the argument "std::shared_ptr" to update the "nullptr" for 
    // the queue's word after "pop" to completely transfer ownership of the 
    // memory to the user.
    template <typename T, 
              bool IsSharePtr = std::is_same<ValueType, std::unique_ptr<T>>::value,
              typename std::enable_if<IsSharePtr>::type* = nullptr>
    std::pair<Error::Code, ValueType> pop() {
        uint32_t index;
        std::pair<Error::Code, ValueType> ret;
        SYNC_READ(index, ret)
        ret.second.swap(reinterpret_cast<ValueType&>(this->buffer[index]));
        return ret;
    }
};

#endif //_UTILS_QUEUE_H_