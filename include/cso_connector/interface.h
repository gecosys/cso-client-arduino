#ifndef _CSO_CONNECTOR_INTERFACE_H_
#define _CSO_CONNECTOR_INTERFACE_H_

#include <Arduino.h>
#include "utils/utils_code.h"

class IConnector {
public:
    // "loopReconnect" should be called in 1 core
    virtual void loopReconnect() = 0;
    // "listen" should be called in other core
    virtual void listen(Error::Code (*cb)(const char* sender, byte* data, uint16_t lenData)) = 0;

    virtual Error::Code sendMessage(
        const char* recvName, 
        byte* content, 
        uint16_t lenContent, 
        bool isEncrypted, 
        bool isCache
    ) = 0;
    virtual Error::Code sendGroupMessage(
        const char* groupName, 
        byte* content, 
        uint16_t lenContent, 
        bool isEncrypted, 
        bool isCache
    ) = 0;

    virtual Error::Code sendMessageAndRetry(
        const char* recvName, 
        byte* content, 
        uint16_t lenContent, 
        bool isEncrypted, 
        int32_t retry
    ) = 0;
    virtual Error::Code sendGroupMessageAndRetry(
        const char* groupName, 
        byte* content, 
        uint16_t lenContent, 
        bool isEncrypted, 
        int32_t retry
    ) = 0;
};

#endif //_CSO_CONNECTOR_INTERFACE_H_