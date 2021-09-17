// #include <lwip/sockets.h>
#include "cso_connection/connection.h"
#include "utils/utils_define.h"

#define HEADER_SIZE 2
#define BUFFER_SIZE 1024

std::unique_ptr<IConnection> Connection::build(uint16_t queueSize) {
    return std::unique_ptr<IConnection>(new Connection(queueSize));
}

Connection::Connection(uint16_t queueSize) 
    : client{},
      status(Status::Disconnected),
      nextMsg{ queueSize } {}

Connection::~Connection() noexcept {
    this->client.stop();
}

Error Connection::connect(const std::string& host, uint16_t port) {
    if (this->status.load(std::memory_order_acquire) == Status::Connected) {
        return Error{};
    }

    uint8_t retry = 0;
    while (!client.connect(host.c_str(), port)) {
        if (++retry >= 2) {
            return Error{ GET_FUNC_NAME(), "Socket connection failed" };
        }
        vTaskDelay(100);
    }
    this->status.store(Status::Connected, std::memory_order_release);
    return Error{};
}

Error Connection::loopListen() {
    bool header{ true };
    // bool disconnected = true;
    uint16_t seek{ 0 };
    uint16_t readed{ 0 };
    uint16_t length{ HEADER_SIZE };
    uint8_t* message{ nullptr };
    std::unique_ptr<uint8_t> buffer{ new uint8_t[BUFFER_SIZE] };

    while (WiFi.status() == WL_CONNECTED && 
           this->status.load(std::memory_order_acquire) == Status::Connected) {
        if (this->client.available() <= 0) {
            // disconnected = false;
            // break;
            delay(50);
            continue;
        }

        // "readed" always is <= "length - seek"
        readed = this->client.readBytes(buffer.get() + seek, length - seek);
        if (readed <= 0) {
            // disconnected = false;
            // break;
            delay(50);
            continue;
        }

        // Read enough data
        seek += readed;
        if (seek < length) {
            continue;
        }

        // Read "data length"
        if (header) {
            length = (buffer.get()[1] << 8U) | buffer.get()[0];
            if (length > 0) {
                header = false;
            }
            seek = 0;
            continue;
        }

        // Build "message"
        // Don't delete "message" because queue will manage memory
        message = new uint8_t[length];
        memcpy(message, buffer.get(), length);
        this->nextMsg.try_push(Array<uint8_t>{ message, length });

        // disconnected = false;
        // break;
        // Reset
        length = HEADER_SIZE;
        header = true;
        seek = 0;
    }

    this->client.stop();
    this->status.store(Status::Disconnected, std::memory_order_release);
    return Error{ GET_FUNC_NAME(), "Socket connection is disconnected" };
    // if (disconnected) {
    //     this->client.stop();        
    //     this->status = Status::Disconnected;
    //     return Error::CSOConnection_Disconnected;
    // }
    // return Error::Nil;
}

Error Connection::sendMessage(const Array<uint8_t>& data) {
    if (WiFi.status() != WL_CONNECTED || 
        this->status.load(std::memory_order_acquire) != Status::Connected) {
        return Error{ GET_FUNC_NAME(), "Socket connection is disconnected" };
    }

    //============
    // Make buffer
    //============
    uint16_t nBytes{ data.length() };
    std::unique_ptr<uint8_t> buffer(new uint8_t[HEADER_SIZE + nBytes]); // Add 2 bytes "data length"
    memcpy(buffer.get(), &nBytes, sizeof(uint16_t));
    memcpy(buffer.get() + HEADER_SIZE, data.get(), nBytes);

    //===============
    // Send and check
    //=============== 
    nBytes += HEADER_SIZE;
    for (size_t seek = 0, sent = 0; seek < nBytes; seek += sent) {
        sent = this->client.write(buffer.get() + seek, nBytes - seek);
        if (sent > 0) {
            continue;
        }

        this->client.stop();
        this->status.store(Status::Disconnected, std::memory_order_release);
        return Error{ GET_FUNC_NAME(), "Socket connection is disconnected" };
    }
    return Error{};
}

Array<uint8_t> Connection::getMessage() {
    bool ok;
    Array<uint8_t> msg;

    std::tie(ok, msg) = this->nextMsg.try_pop();
    if (ok) {
        return msg;
    }
    return Array<uint8_t>{};
}