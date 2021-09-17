#ifndef ENTITY_SPSC_QUEUE_HPP
#define ENTITY_SPSC_QUEUE_HPP

#include <tuple>
#include <atomic>
#include <cstdint>
#include <cassert>
#include <type_traits>

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

        Type&& get() noexcept {
            return std::move(*(reinterpret_cast<Type*>(std::addressof(this->storage))));
        }
    };

private:
    private:
    size_t capacity;
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

    constexpr size_t idx(size_t i) const noexcept { return i & (this->capacity - 1U); }
    constexpr size_t turn(size_t i) const noexcept { return i / this->capacity; }

public:
    SPSCQueue(SPSCQueue&& other) = delete;
    SPSCQueue(const SPSCQueue& other) = delete;

    SPSCQueue& operator=(SPSCQueue&& other) = delete;
    SPSCQueue& operator=(const SPSCQueue& other) = delete;

    explicit SPSCQueue(const size_t capacity)
        : capacity{ 0 },
          slots{ nullptr },
          head{ 0 },
          tail{ 0 } {
        static_assert(
            std::is_nothrow_default_constructible<Type>::value &
            (std::is_nothrow_move_constructible<Type>::value ||
            std::is_nothrow_copy_constructible<Type>::value),
            "Type must get noexcept copy constructor or move constructor + move assign"
        );

        static_assert(
            std::is_nothrow_destructible<Type>::value,
            "Type must be noexcept destructor"
        );

        if (capacity == 0) {
            throw "Capacity must be larger than 0";
        }

        this->capacity = roundupToPowerOf2(capacity);
        this->slots = new Slot[this->capacity];
    }

    ~SPSCQueue() noexcept {
        for (size_t i = 0; i < this->capacity; ++i) {
            this->slots[i].~Slot();
        }
        delete[] this->slots;
    }

    template <typename... Args>
    bool try_push(Args &&... args) noexcept {
        auto tail = this->tail;
        auto turn = this->turn(tail) << 1U;
        auto& slot = this->slots[idx(tail)];

        if (turn != slot.turn.load(std::memory_order_acquire)) {
            return false;
        }

        this->tail++;
        slot.construct(std::forward<Args>(args)...);
        slot.turn.store(turn + 1U, std::memory_order_release);
        return true;
    }

    std::tuple<bool, Type> try_pop() noexcept {
        auto head = this->head;
        auto turn = (this->turn(head) << 1U) + 1U;
        auto& slot = this->slots[idx(head)];

        if (turn != slot.turn.load(std::memory_order_acquire)) {
            return std::make_tuple(false, Type{});
        }

        // Get result & Update slot
        std::tuple<bool, Type> result{ true, slot.get() };
        slot.destroy();
        slot.turn.store(turn + 1U, std::memory_order_release);
        return result;
    }
};

#endif // !ENTITY_SPSC_QUEUE_H