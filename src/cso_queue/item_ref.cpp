#include "cso_queue/item_ref.h"

ItemQueueRef::ItemQueueRef(std::shared_ptr<ItemQueue>&& ptr) noexcept
    : ptr{ std::forward<std::shared_ptr<ItemQueue>>(ptr) } {}

bool ItemQueueRef::empty() const noexcept {
    return this->ptr == nullptr;
}

ItemQueue& ItemQueueRef::get() const noexcept {
    return *(this->ptr);
}