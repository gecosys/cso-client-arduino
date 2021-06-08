#ifndef _CSO_QUEUE_H_
#define _CSO_QUEUE_H_

#include <atomic>
#include "interface.h"

class Queue : public IQueue {
private:
    uint32_t capacity;
    std::atomic<uint32_t> length;
    std::unique_ptr<ItemQueue>* items;

public:
    static std::unique_ptr<IQueue> build(uint32_t capacity);

private:
    Queue(uint32_t capacity);

public:
    Queue() = delete;
    Queue(Queue&& other) = delete;
    Queue(const Queue& other) = delete;
    Queue& operator=(const Queue& other) = delete;

    ~Queue() noexcept;

    // Method can invoke on many threads
	// This method needs to be invoked before PushMessage method
	bool takeIndex() noexcept;
    void pushMessage(ItemQueue* item) noexcept;
    ItemQueueRef nextMessage() noexcept;
    void clearMessage(uint64_t msgID) noexcept;
};

#endif //_CSO_QUEUE_H_