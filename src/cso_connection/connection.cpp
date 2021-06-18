// #include <lwip/sockets.h>
#include <new>
#include "cso_connection/connection.h"

#define HEADER_SIZE 2
#define BUFFER_SIZE 1024

std::unique_ptr<IConnection> Connection::build(uint16_t queueSize) {
    return std::unique_ptr<IConnection>(new Connection(queueSize));
}

Connection::Connection(uint16_t queueSize) 
    : nextMessage(queueSize),
      status(Status::Prepare) {}

Connection::~Connection() noexcept {
    this->client.stop();
}

Error::Code Connection::connect(const char* host, uint16_t port) {
    if (this->status.load() == Status::Connected) {
        return Error::Nil;
    }
    uint8_t retry = 0;
    while (!client.connect(host, port)) {
        if (++retry >= 2) {
            return Error::CSOConnection_Disconnected;
        }
        vTaskDelay(100);
    }

    if (!setup()) {
        return Error::CSOConnection_SetupFailed;
    }
    this->status.store(Status::Connected);
    return Error::Nil;
}

Error::Code Connection::loopListen() {
    std::unique_ptr<uint8_t> buffer(new (std::nothrow) uint8_t[BUFFER_SIZE]);
    if (buffer == nullptr) {
        return Error::NotEnoughMemory;
    }

    bool header = true;
    // bool disconnected = true;
    uint8_t* message = nullptr;
    uint16_t seek = 0;
    uint16_t readed = 0;
    uint16_t length = HEADER_SIZE;

    while (WiFi.status() == WL_CONNECTED && this->status.load() == Status::Connected) {
        if (this->client.available() <= 0) {
            // disconnected = false;
            // break;
            vTaskDelay(50 / portTICK_PERIOD_MS);
            continue;
        }

        // "readed" always is <= "length - seek"
        readed = this->client.readBytes(buffer.get() + seek, length - seek);
        if (readed <= 0) {
            // disconnected = false;
            // break;
            vTaskDelay(50 / portTICK_PERIOD_MS);
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
        message = new (std::nothrow) uint8_t[length];
        if (message == nullptr) {
            return Error::NotEnoughMemory;
        }

        // Push message
        // Don't delete "message" because queue will manage memory
        memcpy(message, buffer.get(), length);
        this->nextMessage.push(Array<uint8_t>(message, length));

        // disconnected = false;
        // break;
        // Reset
        length = HEADER_SIZE;
        header = true;
        seek = 0;
    }
    this->client.stop();
    this->status.store(Status::Disconnected);
    return Error::CSOConnection_Disconnected;
    // if (disconnected) {
    //     this->client.stop();        
    //     this->status = Status::Disconnected;
    //     return Error::CSOConnection_Disconnected;
    // }
    // return Error::Nil;
}

Error::Code Connection::sendMessage(uint8_t* data, uint16_t nBytes) {
    if (WiFi.status() != WL_CONNECTED || this->status.load() != Status::Connected) {
        this->status = Status::Disconnected;
        return Error::CSOConnection_Disconnected;
    }

    //============
    // Make buffer
    //============
    std::unique_ptr<uint8_t> buffer(new (std::nothrow) uint8_t[nBytes + HEADER_SIZE]); // Add 2 bytes "data length"
    if (buffer == nullptr) {
        return Error::NotEnoughMemory;
    }
    memcpy(buffer.get(), &nBytes, sizeof(uint16_t));
    memcpy(buffer.get() + HEADER_SIZE, data, nBytes);

    //===============
    // Send and check
    //=============== 
    nBytes += HEADER_SIZE;
    for (size_t seek = 0, sent = 0; seek < nBytes; seek += sent) {
        sent = this->client.write(buffer.get() + seek, nBytes - seek);
        if (sent == 0) {
            this->client.stop();
            this->status.store(Status::Disconnected);
            return Error::CSOConnection_Disconnected;
        }
    }
    return Error::Nil;
}

Array<uint8_t> Connection::getMessage() {
    return this->nextMessage.pop().data;
}

bool Connection::setup() noexcept {
    // timeout 20s for read + write
    if (this->client.setTimeout(20) != ESP_OK) {
        return false;
    }

    // Keep connection
    int32_t val = 1;
    if (this->client.setSocketOption(0x0008, (char*)&val, sizeof(int32_t)) != ESP_OK) {
        return false;
    }
    return true;
}