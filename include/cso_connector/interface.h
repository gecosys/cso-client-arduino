#ifndef CSO_CONNECTOR_INTERFACE_H
#define CSO_CONNECTOR_INTERFACE_H

#include "entity/array.h"
#include "error/error.h"

class IConnector {
public:
    // "loopReconnect" should be called in core 1 of esp32
    virtual void loopReconnect() = 0;
    // "listen" should be called in core 0 of esp32
    virtual void listen(Error::Code (*cb)(const std::string& sender, const Array<uint8_t>& data)) = 0;

    virtual Error::Code sendMessage(const std::string& recvName, const Array<uint8_t>& content, bool isEncrypted, bool isCache) = 0;
    virtual Error::Code sendGroupMessage(const std::string& groupName, const Array<uint8_t>& content, bool isEncrypted, bool isCache) = 0;

    virtual Error::Code sendMessageAndRetry(const std::string& recvName, const Array<uint8_t>& content, bool isEncrypted, int32_t retry) = 0;
    virtual Error::Code sendGroupMessageAndRetry(const std::string& groupName, const Array<uint8_t>& content, bool isEncrypted, int32_t retry) = 0;
};

#endif // !CSO_CONNECTOR_INTERFACE_H