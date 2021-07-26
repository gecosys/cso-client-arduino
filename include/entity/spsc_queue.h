#ifndef ENTITY_SPSC_QUEUE_H
#define ENTITY_SPSC_QUEUE_H

#include <tuple>
#include <atomic>
#include <cstdint>
#include <cassert>
#include <type_traits>
#include "error/error.h"
#include "error/package/entity_err.h"

template<typename Type>
class SPSCQueue {
private:
    struct Slot {
        std::atomic<size_t> turn{ 0 };
        typename std::aligned_storage<sizeof(Type), alignof(Type)>::type storage;

        ~Slot() noexcept {
            // "turn" is odd number that slot is written (or has data)
            if (this->turn.load(std::memory_order_acquire) & 1) {
                destroy();
            }
        }

        template<typename... Args>
        void construct(Args &&... args) noexcept {
            new (&this->storage) Type(std::forward<Args>(args)...);
        }

        void destroy() noexcept {
            reinterpret_cast<Type*>(std::addressof(this->storage))->~Type();
        }

        Type& reference() noexcept {
            return *(reinterpret_cast<Type*>(std::addressof(this->storage)));
        }

        Type&& move() noexcept {
            return std::move(*(reinterpret_cast<Type*>(std::addressof(this->storage))));
        }
    };

private:
    private:
    size_t cap;
    Slot* slots;
    size_t head;
    size_t tail;

private:
    constexpr size_t roundupToPowerOf2(size_t n) const noexcept {
        --n;
        n |= n >> 1U;
        n |= n >> 2U;
        n |= n >> 4U;
        n |= n >> 8U;
        n |= n >> 16U;
        return ++n;
    }

    constexpr size_t idx(size_t i) const noexcept { return i & (this->cap - 1U); }
    constexpr size_t turn(size_t i) const noexcept { return i / this->cap; }

public:
    SPSCQueue(SPSCQueue&& other) = delete;
    SPSCQueue(const SPSCQueue& other) = delete;

    SPSCQueue& operator=(SPSCQueue&& other) = delete;
    SPSCQueue& operator=(const SPSCQueue& other) = delete;

    explicit SPSCQueue(const size_t capacity)
        : cap{ 0 },
          slots{ nullptr },
          head{ 0 },
          tail{ 0 } {
        static_assert(
            std::is_nothrow_move_constructible<Type>::value ||
            std::is_nothrow_copy_constructible<Type>::value,
            "Type must get noexcept copy constructor or move constructor + move assign"
        );

        static_assert(
            std::is_nothrow_destructible<Type>::value,
            "Type must be noexcept destructor"
        );

        if (capacity == 0) {
            throw "Capacity must be larger than 0";
        }

        this->cap = roundupToPowerOf2(capacity);
        this->slots = new Slot[this->cap];
    }

    ~SPSCQueue() noexcept {
        for (size_t i = 0; i < this->cap; ++i) {
            this->slots[i].~Slot();
        }
        delete[] this->slots;
    }

    template <typename... Args>
    Error::Code try_push(Args &&... args) noexcept {
        auto tail = this->tail++;
        auto turn = this->turn(tail) << 1U;
        auto& slot = this->slots[idx(tail)];

        if (turn != slot.turn.load(std::memory_order_acquire)) {
            this->tail--;
            return Error::buildCode(
                EntityErr::ID,
                EntityErr::Func::SPSCQueue_TryPush,
                EntityErr::Reason::SPSCQueue_QueueFull
            );
        }

        slot.construct(std::forward<Args>(args)...);
        slot.turn.store(turn + 1U, std::memory_order_release);
        return Error::Code::Nil;
    }

    std::tuple<Error::Code, Type> try_pop() noexcept {
        auto head = this->head++;
        auto turn = (this->turn(head) << 1U) + 1U;
        auto& slot = this->slots[idx(head)];

        if (turn != slot.turn.load(std::memory_order_acquire)) {
            this->head--;
            return std::make_tuple(
                Error::buildCode(
                    EntityErr::ID,
                    EntityErr::Func::SPSCQueue_TryPop,
                    EntityErr::Reason::SPSCQueue_QueueEmpty
                ),
                Type{}
            );
        }

        // Get result
        std::tuple<Error::Code, Type> result;
        if (std::is_nothrow_move_constructible<Type>::value) {
            result = std::make_tuple(Error::Code::Nil, slot.move());
        }
        // Because Queue's constructor checked object that must get noexcept copy or move constructible,
        // if not move constructible, is copy constructible
        else {
            result = std::make_tuple(Error::Code::Nil, slot.reference());
        }

        // Update slot
        slot.destroy();
        slot.turn.store(turn + 1U, std::memory_order_release);
        return result;
    }
};

#endif // !ENTITY_SPSC_QUEUE_H