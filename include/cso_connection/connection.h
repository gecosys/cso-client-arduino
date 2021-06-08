#ifndef _CSO_CONNECTION_H_
#define _CSO_CONNECTION_H_

#include <WiFi.h>
#include <atomic>
#include "status.h"
#include "interface.h"
#include "synchronization/concurrency_queue.h"

class Connection : public IConnection {
private:
    ConcurrencyQueue<Array<byte>> nextMessage;
    std::atomic<uint8_t> status;
    WiFiClient client;

public:
    static std::unique_ptr<IConnection> build(uint16_t queueSize);

private:
    Connection(uint16_t queueSize);
    
    bool setup() noexcept;

public:
    Connection() = delete;
    Connection(Connection&& other) = delete;
    Connection(const Connection& other) = delete;
    Connection& operator=(const Connection& other) = delete;

    ~Connection() noexcept;

    Error::Code connect(const char* host, uint16_t port);
    Error::Code loopListen();
    Error::Code sendMessage(byte* data, uint16_t nBytes);
    Array<byte> getMessage();
};

#endif //_CSO_CONNECTION_H_