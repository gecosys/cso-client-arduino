#include "cso_queue/item_ref.h"

ItemQueueRef::ItemQueueRef(ItemQueue* ptr) noexcept
    : ptr(ptr) {}

ItemQueueRef::~ItemQueueRef() noexcept {}

bool ItemQueueRef::empty() noexcept {
    return this->ptr == nullptr;
}

ItemQueue& ItemQueueRef::get() const noexcept {
    return *(this->ptr);
}