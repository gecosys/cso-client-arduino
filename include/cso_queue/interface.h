#ifndef CSO_QUEUE_INTERFACE_H
#define CSO_QUEUE_INTERFACE_H

#include <memory>
#include "item.h"
#include "item_ref.h"

class IQueue {
public:
    // Method can invoke on many threads
    // This method needs to be invoked before PushMessage method
    virtual bool takeIndex() noexcept = 0;
    virtual void pushMessage(std::unique_ptr<ItemQueue>&& item) noexcept = 0;
    virtual ItemQueueRef nextMessage() noexcept = 0;
    virtual void clearMessage(uint64_t msgID) noexcept = 0;
};

#endif // !CSO_QUEUE_INTERFACE_H