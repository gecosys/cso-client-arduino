#ifndef _CSO_QUEUE_ITEM_H_
#define _CSO_QUEUE_ITEM_H_

#include <memory>
#include <string>
#include <cstdint>
#include <WString.h>
#include "utils/array.h"

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

    ItemQueue() noexcept
        : msgID(-1),
          msgTag(-1),
          recvName(""),
          content(),
          isEncrypted(false),
          isCached(false),
          isFirst(false),
          isLast(false),
          isRequest(false),
          isGroup(false),
          numberRetry(0),
          timestamp(0) {}

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
    ) noexcept 
        : msgID(msgID),
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