#ifndef _CSO_CONNECTION_INTERFACE_H_
#define _CSO_CONNECTION_INTERFACE_H_

#include "utils/array.h"
#include "error/error_code.h"

class IConnection {
public:
    virtual Error::Code connect(const char* host, uint16_t port) = 0;
    virtual Error::Code loopListen() = 0;
    virtual Error::Code sendMessage(uint8_t* data, uint16_t nBytes) = 0;
    virtual Array<uint8_t> getMessage() = 0;
};

#endif // _CSO_CONNECTION_INTERFACE_H_