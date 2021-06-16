#ifndef _CSO_CONNECTOR_INTERFACE_H_
#define _CSO_CONNECTOR_INTERFACE_H_

#include "error/error_code.h"

class IConnector {
public:
    // "loopReconnect" should be called in core 1 of esp32
    virtual void loopReconnect() = 0;
    // "listen" should be called in core 0 of esp32
    virtual void listen(Error::Code (*cb)(const char* sender, uint8_t* data, uint16_t lenData)) = 0;

    virtual Error::Code sendMessage(const char* recvName, uint8_t* content, uint16_t lenContent, bool isEncrypted, bool isCache) = 0;
    virtual Error::Code sendGroupMessage(const char* groupName, uint8_t* content, uint16_t lenContent, bool isEncrypted, bool isCache) = 0;
    virtual Error::Code sendMessageAndRetry(const char* recvName, uint8_t* content, uint16_t lenContent, bool isEncrypted, int32_t retry) = 0;
    virtual Error::Code sendGroupMessageAndRetry(const char* groupName, uint8_t* content, uint16_t lenContent, bool isEncrypted, int32_t retry) = 0;
};

#endif //_CSO_CONNECTOR_INTERFACE_H_