#ifndef _CSO_CONNECTION_H_
#define _CSO_CONNECTION_H_

#include <WiFi.h>
#include "status.h"
#include "interface.h"
#include "utils/utils_safe.h"
#include "utils/utils_concurrency_queue.h"

class Connection : public IConnection {
private:
    ConcurrencyQueue<Array<byte>> nextMessage;
    WiFiClient client;
    Status::Code status;

public:
    static std::shared_ptr<IConnection> build(uint16_t queueSize);

private:
    friend class Safe;
    Connection(uint16_t queueSize);

public:
    Connection() = delete;
    Connection(Connection&& other) = delete;
    Connection(const Connection& other) = delete;
    virtual ~Connection() noexcept;

    Error::Code connect(const char* host, uint16_t port);
    Error::Code loopListen();
    Error::Code sendMessage(byte* data, uint16_t nBytes);
    Array<byte> getMessage();
};

#endif //_CSO_CONNECTION_H_