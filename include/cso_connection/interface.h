#ifndef CSO_CONNECTION_INTERFACE_H
#define CSO_CONNECTION_INTERFACE_H

#include "entity/array.h"
#include "error/error.h"

class IConnection {
public:
    virtual Error::Code connect(const std::string& host, uint16_t port) = 0;
    virtual Error::Code loopListen() = 0;
    virtual Error::Code sendMessage(const Array<uint8_t>& data) = 0;
    virtual Array<uint8_t> getMessage() = 0;
};

#endif // !CSO_CONNECTION_INTERFACE_H