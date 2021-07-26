#include <cstdio>
#include "cso_queue/item.h"

ItemQueue::ItemQueue() noexcept
  : msgID{ 0 },
    msgTag{ 0 },
    recvName{ "" },
    content{},
    isEncrypted{ false },
    isCached{ false },
    isFirst{ false },
    isLast{ false },
    isRequest{ false },
    isGroup{ false },
    numberRetry{ 0 },
    timestamp{ 0 } {}

ItemQueue::ItemQueue(ItemQueue&& other) noexcept
  : msgID{ std::move(other.msgID) },
    msgTag{ std::move(other.msgTag) },
    recvName{ std::move(other.recvName) },
    content{ std::move(other.content) },
    isEncrypted{ std::move(other.isEncrypted) },
    isCached{ std::move(other.isCached) },
    isFirst{ std::move(other.isFirst) },
    isLast{ std::move(other.isLast) },
    isRequest{ std::move(other.isRequest) },
    isGroup{ std::move(other.isGroup) },
    numberRetry{ std::move(other.numberRetry) },
    timestamp{ std::move(other.timestamp) } {}

ItemQueue& ItemQueue::operator=(ItemQueue&& other) noexcept {
  this->msgID = std::move(other.msgID);
  this->msgTag = std::move(other.msgTag);
  this->recvName = std::move(other.recvName);
  this->content = std::move(other.content);
  this->isEncrypted = std::move(other.isEncrypted);
  this->isCached = std::move(other.isCached);
  this->isFirst = std::move(other.isFirst);
  this->isLast = std::move(other.isLast);
  this->isRequest = std::move(other.isRequest);
  this->isGroup = std::move(other.isGroup);
  this->numberRetry = std::move(other.numberRetry);
  this->timestamp = std::move(other.timestamp);
  return *this;
}