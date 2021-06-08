#include <esp_timer.h>
#include "cso_queue/queue.h"

std::unique_ptr<IQueue> Queue::build(uint32_t capacity) {
    IQueue* obj = new (std::nothrow) Queue(capacity);
    if (obj == nullptr) {
        throw std::runtime_error("[cso_queue/Queue::build(...)]Not enough memory to create object");
    }
    return std::unique_ptr<IQueue>(obj);
}

Queue::Queue(uint32_t cap) 
    : capacity(cap),
      length(0) {
    this->items = new (std::nothrow) std::unique_ptr<ItemQueue>[this->capacity];
    if (this->items == nullptr) {
        throw std::runtime_error("[cso_queue/Queue(...)]Not enough memory to create array");
    }
}

Queue::~Queue() {
    delete[] this->items;
}

// Method can invoke on many threads
// This method needs to be invoked before PushMessage method
bool Queue::takeIndex() noexcept {
    if (this->length.fetch_add(1) < this->capacity) {
        return true;
    }
    this->length.fetch_sub(1);
    return false;
}

void Queue::pushMessage(ItemQueue* item) noexcept {
    for (uint32_t idx = 0; idx < this->capacity; ++idx) {
        if (this->items[idx] == nullptr) {
            this->items[idx].reset(item);
            break;
        }
    }
}

ItemQueueRef Queue::nextMessage() noexcept {
    ItemQueue* nextItem = nullptr;
    uint64_t now = esp_timer_get_time() / 1000000ULL; // (seconds)
    for (uint32_t idx = 0; idx < this->capacity; ++idx) {
        if (this->items[idx] == nullptr) {
           continue;
        }
        if (nextItem == nullptr && (now - this->items[idx]->timestamp) >= 3) {
            nextItem = this->items[idx].get();
            nextItem->timestamp = now;
            nextItem->numberRetry--;
        }
        if (this->items[idx]->numberRetry == 0) {
            this->items[idx].reset();
            this->length.fetch_sub(1);
        }
    }
    return ItemQueueRef(nextItem);
}

void Queue::clearMessage(uint64_t msgID) noexcept {
    if (msgID >= this->capacity) {
        return;
    }
    if (this->items[msgID].get() == nullptr) {
        return;
    }
    this->items[msgID].reset();
    this->length.fetch_sub(1);
}