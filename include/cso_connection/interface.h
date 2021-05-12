#ifndef _CSO_CONNECTION_INTERFACE_H_
#define _CSO_CONNECTION_INTERFACE_H_

#include <Arduino.h>
#include <message/array.h>
#include "utils/utils_code.h"

class IConnection {
public:
    virtual Error::Code connect(const char* ssid, const char* pswd, const char* host, uint16_t port) = 0;
    virtual Error::Code loopListen() = 0;
    virtual Error::Code sendMessage(byte* data, uint16_t nBytes) = 0;
    virtual Array<byte> getMessage() = 0;
};

#endif // _CSO_CONNECTION_INTERFACE_H_