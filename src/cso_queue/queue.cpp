#include <chrono>
#include <mutex>
#include "cso_queue/queue.h"

std::unique_ptr<IQueue> Queue::build(uint32_t capacity) {
    return std::unique_ptr<IQueue>{ new Queue{ capacity } };
}

Queue::Queue(uint32_t cap)
    : lock{},
      capacity{ cap },
      length{ 0 },
      items{ new std::shared_ptr<ItemQueue>[this->capacity] } {}

Queue::~Queue() {
    delete[] this->items;
}

// Method can invoke on many threads
// This method needs to be invoked before PushMessage method
bool Queue::takeIndex() noexcept {
    if (this->length.fetch_add(1, std::memory_order_relaxed) < this->capacity) {
        return true;
    }
    this->length.fetch_sub(1, std::memory_order_relaxed);
    return false;
}

void Queue::pushMessage(std::unique_ptr<ItemQueue>&& item) noexcept {
    for (uint32_t idx = 0; idx < this->capacity; ++idx) {
        this->lock.lock();
        if (this->items[idx] != nullptr) {
            this->lock.unlock();
            continue;
        }
        this->items[idx].reset(item.release());
        this->lock.unlock();
        return;
    }
}

ItemQueueRef Queue::nextMessage() noexcept {
    using namespace std::chrono;
    std::shared_ptr<ItemQueue> nextItem{ nullptr };
    uint64_t now = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();

    for (uint32_t idx = 0; idx < this->capacity; ++idx) {
        this->lock.lock();
        if (this->items[idx] == nullptr) {
            this->lock.unlock();
            continue;
        }
        if (nextItem == nullptr && (now - this->items[idx]->timestamp) >= 3) {
            nextItem = this->items[idx];
            nextItem->timestamp = now;
            nextItem->numberRetry--;
        }
        if (this->items[idx]->numberRetry == 0) {
            this->items[idx].reset();
            this->length.fetch_sub(1, std::memory_order_relaxed);
        }
        this->lock.unlock();
    }
    return ItemQueueRef(std::move(nextItem));
}

void Queue::clearMessage(uint64_t msgID) noexcept {
    for (uint64_t idx = 0; idx < this->capacity; ++idx) {
        this->lock.lock();
        if (this->items[idx] == nullptr || this->items[idx]->msgID != msgID) {
            this->lock.unlock();
            continue;
        }
        this->items[idx].reset();
        this->lock.unlock();
        this->length.fetch_sub(1, std::memory_order_relaxed);
        return;
    }
}