#ifndef CSO_QUEUE_ITEM_H
#define CSO_QUEUE_ITEM_H

#include <string>
#include "error/error.h"
#include "entity/array.h"

struct ItemQueue {
    uint64_t msgID;
    uint64_t msgTag;
    std::string recvName;
    Array<uint8_t> content;
    bool isEncrypted;
    bool isCached;
    bool isFirst;
    bool isLast;
    bool isRequest;
    bool isGroup;
    uint32_t numberRetry;
    uint64_t timestamp;

    ItemQueue() noexcept;
    ItemQueue(ItemQueue&& other) noexcept;
    ItemQueue(const ItemQueue& other) = default;
    ~ItemQueue() noexcept = default;

    ItemQueue& operator=(const ItemQueue& other) = default;
    ItemQueue& operator=(ItemQueue&& other) noexcept;
};

#endif // !CSO_QUEUE_ITEM_H