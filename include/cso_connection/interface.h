#ifndef CSO_CONNECTION_INTERFACE_H
#define CSO_CONNECTION_INTERFACE_H

#include "error/error.h"
#include "entity/array.hpp"

class IConnection {
public:
    virtual Error connect(const std::string& host, uint16_t port) = 0;
    virtual Error loopListen() = 0;
    virtual Error sendMessage(const Array<uint8_t>& data) = 0;
    virtual Array<uint8_t> getMessage() = 0;
};

#endif // !CSO_CONNECTION_INTERFACE_H