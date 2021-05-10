#ifndef _QUEUE_ITEM_H_
#define _QUEUE_ITEM_H_

#include <memory>
#include <string>
#include <cstdint>
#include "message/array.h"

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
};

#endif //_QUEUE_ITEM_H_