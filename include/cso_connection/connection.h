#ifndef CSO_CONNECTION_H
#define CSO_CONNECTION_H

#include <WiFi.h>
#include <atomic>
#include "status.h"
#include "interface.h"
#include "entity/spsc_queue.h"

class Connection : public IConnection {
private:
    WiFiClient client;
    std::atomic<uint8_t> status;
    SPSCQueue<Array<uint8_t>> nextMsg;

public:
    static std::unique_ptr<IConnection> build(uint16_t queueSize);

private:
    Connection(uint16_t queueSize);

public:
    Connection() = delete;
    Connection(Connection&& other) = delete;
    Connection(const Connection& other) = delete;
    ~Connection() noexcept;

    Connection& operator=(Connection&& other) = delete;
    Connection& operator=(const Connection& other) = delete;

    Error::Code connect(const std::string& host, uint16_t port);
    Error::Code loopListen();
    Error::Code sendMessage(const Array<uint8_t>& data);
    Array<uint8_t> getMessage();
};

#endif // !CSO_CONNECTION_H