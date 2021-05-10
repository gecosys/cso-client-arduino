#include <esp_timer.h>
#include "queue/queue.h"

std::shared_ptr<IQueue> Queue::build(uint32_t capacity) {
    return std::shared_ptr<IQueue>(Safe::new_obj<Queue>(capacity));
}

Queue::Queue(uint32_t cap) 
    : capacity(cap),
      length(0) {
    this->items = Safe::new_arr<std::shared_ptr<ItemQueue>>(this->capacity);
    if (this->items == nullptr) {
        throw "[queue/Queue(uint32_t capacity)]Not enough mem";
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

void Queue::pushMessage(std::shared_ptr<ItemQueue> item) noexcept {
    for (uint32_t idx = 0; idx < this->capacity; ++idx) {
        if (this->items[idx].get() == nullptr) {
            this->items[idx].swap(item);
            break;
        }
    }
}

const std::shared_ptr<ItemQueue>* Queue::nextMessage() noexcept {
    std::shared_ptr<ItemQueue>* nextItem = nullptr;
    uint64_t now = esp_timer_get_time() / 1000000ULL; // (seconds)
    for (uint32_t idx = 0; idx < this->capacity; ++idx) {
        if (this->items[idx].get() == nullptr) {
           continue;
        }
        if (nextItem == nullptr && (now - this->items[idx]->timestamp) >= 3) {
            nextItem = &this->items[idx];
            (*nextItem)->timestamp = now;
            (*nextItem)->numberRetry--;
        }
        if (this->items[idx]->numberRetry == 0) {
            this->items[idx].reset();
            this->length.fetch_sub(1);
        }
    }
    return nextItem;
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