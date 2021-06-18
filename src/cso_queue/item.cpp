#include "cso_queue/item.h"

ItemQueue::ItemQueue() noexcept
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

ItemQueue::ItemQueue(
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

ItemQueue::ItemQueue(ItemQueue&& other) noexcept
  : msgID(other.msgID),
    msgTag(other.msgTag),
    recvName(std::move(other.recvName)),
    content(std::move(other.content)),
    isEncrypted(other.isEncrypted),
    isCached(other.isCached),
    isFirst(other.isFirst),
    isLast(other.isLast),
    isRequest(other.isRequest),
    isGroup(other.isGroup),
    numberRetry(other.numberRetry),
    timestamp(other.timestamp) {}

ItemQueue& ItemQueue::operator=(ItemQueue&& other) noexcept {
    this->msgID = other.msgID;
    this->msgTag = other.msgTag;
    std::swap(this->recvName, other.recvName);
    std::swap(this->content, other.content);
    this->isEncrypted = other.isEncrypted;
    this->isCached = other.isCached;
    this->isFirst = other.isFirst;
    this->isLast = other.isLast;
    this->isRequest = other.isRequest;
    this->isGroup = other.isGroup;
    this->numberRetry = other.numberRetry;
    this->timestamp = other.timestamp;
    return *this;
}

Error::Code ItemQueue::copy(const ItemQueue& other) noexcept {
    this->msgID = other.msgID;
    this->msgTag = other.msgTag;
    this->recvName = other.recvName;
    auto errorCode = this->content.copy(other.content);
    if (errorCode != Error::Nil) {
        return errorCode;
    }
    this->isEncrypted = other.isEncrypted;
    this->isCached = other.isCached;
    this->isFirst = other.isFirst;
    this->isLast = other.isLast;
    this->isRequest = other.isRequest;
    this->isGroup = other.isGroup;
    this->numberRetry = other.numberRetry;
    this->timestamp = other.timestamp;
    return Error::Nil;
}