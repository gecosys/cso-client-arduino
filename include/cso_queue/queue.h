#ifndef CSO_QUEUE_H
#define CSO_QUEUE_H

#include <atomic>
#include <memory>
#include "item.h"
#include "interface.h"
#include "entity/spin_lock.h"

class Queue : public IQueue {
private:
    SpinLock lock;
    uint32_t capacity;
    std::atomic<uint32_t> length;
    std::shared_ptr<ItemQueue>* items;

public:
    static std::unique_ptr<IQueue> build(uint32_t capacity);

private:
    Queue(uint32_t capacity);

public:
    Queue() = delete;
    Queue(Queue&& other) = delete;
    Queue(const Queue& other) = delete;
    ~Queue() noexcept;

    Queue& operator=(Queue&& other) = delete;
    Queue& operator=(const Queue& other) = delete;


    // Method can invoke on many threads
    // This method needs to be invoked before PushMessage method
    bool takeIndex() noexcept;
    void pushMessage(std::unique_ptr<ItemQueue>&& item) noexcept;
    ItemQueueRef nextMessage() noexcept;
    void clearMessage(uint64_t msgID) noexcept;
};

#endif // !_CSO_QUEUE_H_