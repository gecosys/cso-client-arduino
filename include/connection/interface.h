#ifndef _CONNECTION_INTERFACE_H_
#define _CONNECTION_INTERFACE_H_

#include <memory>
#include <Arduino.h>
#include "utils/utils_code.h"

class IConnection {
public:
    virtual Error::Code connect(char* ssid, char* pswd, char* host, uint16_t port) = 0;
    virtual Error::Code loopListen() = 0;
    virtual Error::Code sendMessage(byte* data, uint16_t nBytes) = 0;
    virtual std::shared_ptr<byte> getMessage() = 0;
};

#endif // _CONNECTION_INTERFACE_H_