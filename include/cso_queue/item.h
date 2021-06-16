#ifndef _CSO_QUEUE_ITEM_H_
#define _CSO_QUEUE_ITEM_H_

#include <string>
#include "utils/array.h"

class ItemQueue {
public:
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

public:
    ItemQueue() noexcept;
    ItemQueue(
        uint64_t msgID,
        uint64_t msgTag,
        const char* recvName,
        uint8_t* content,
        uint16_t lenContent,
        bool isEncrypted,
        bool isCached,
        bool isFirst,
        bool isLast,
        bool isRequest,
        bool isGroup,
        uint32_t numberRetry,
        uint64_t timestamp
    ) noexcept;
    ItemQueue(ItemQueue&& other) noexcept;
    ItemQueue(const ItemQueue& other) = delete;

    ItemQueue& operator=(const ItemQueue& other) = delete;
    ItemQueue& operator=(ItemQueue&& other) noexcept;

    Error::Code copy(const ItemQueue& other) noexcept;
};

#endif //_CSO_QUEUE_ITEM_H_