#ifndef _CSO_QUEUE_H_
#define _CSO_QUEUE_H_

#include <atomic>
#include "interface.h"
#include "utils/utils_safe.h"

class Queue : public IQueue {
private:
    uint32_t capacity;
    std::atomic<uint32_t> length;
    std::shared_ptr<ItemQueue>* items;

public:
    static std::shared_ptr<IQueue> build(uint32_t capacity);

private:
    friend class Safe;
    Queue() = default;
    Queue(uint32_t capacity);

public:
    Queue(Queue&& other) = delete;
    Queue(const Queue& other) = delete;
    virtual ~Queue() noexcept;

    // Method can invoke on many threads
	// This method needs to be invoked before PushMessage method
	bool takeIndex() noexcept;
    void pushMessage(std::shared_ptr<ItemQueue> item) noexcept;
    const std::shared_ptr<ItemQueue>* nextMessage() noexcept;
    void clearMessage(uint64_t msgID) noexcept;
};

#endif //_CSO_QUEUE_H_