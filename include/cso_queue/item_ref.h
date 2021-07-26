#ifndef CSO_QUEUE_ITEM_REF_H
#define CSO_QUEUE_ITEM_REF_H

#include <memory>
#include "item.h"

class ItemQueueRef {
private:
    std::shared_ptr<ItemQueue> ptr;

public:
    ItemQueueRef() = delete;
    ItemQueueRef(std::shared_ptr<ItemQueue>&& ptr) noexcept;
    ~ItemQueueRef() noexcept = default;

    bool empty() const noexcept;
    ItemQueue& get() const noexcept;
};

#endif // !CSO_QUEUE_ITEM_REF_H
