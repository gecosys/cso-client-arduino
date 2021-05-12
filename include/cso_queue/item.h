#ifndef _CSO_QUEUE_ITEM_H_
#define _CSO_QUEUE_ITEM_H_

#include <memory>
#include <cstdint>
#include <WString.h>
#include "message/array.h"

struct ItemQueue {
    uint64_t msgID;
    uint64_t msgTag;
    String recvName;
    Array<uint8_t> content;
    bool isEncrypted;
    bool isCached;
    bool isFirst;
    bool isLast;
    bool isRequest;
    bool isGroup;
    uint32_t numberRetry;
    uint64_t timestamp;

    ItemQueue() = default;
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
    ) : msgID(msgID),
        msgTag(msgTag),
        recvName(recvName),
        content(content, lenContent),
        isEncrypted(isEncrypted),
        isCached(isCached),
        isFirst(isFirst),
        isLast(isLast),
        isRequest(isRequest),
        isGroup(isGroup),
        numberRetry(numberRetry),
        timestamp(timestamp) {}
};

#endif //_CSO_QUEUE_ITEM_H_