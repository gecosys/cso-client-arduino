#include "item.h"

class ItemQueueRef {
private:
    ItemQueue* ptr;

public:
    ItemQueueRef() = delete;
    ItemQueueRef& operator=(const ItemQueueRef& other) = delete;

    ItemQueueRef(ItemQueue* ptr) noexcept;
    ~ItemQueueRef() noexcept;

    bool empty() const noexcept;
    ItemQueue& get() const noexcept;
};