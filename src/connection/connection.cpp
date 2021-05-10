#include "connection/connection.h"

std::shared_ptr<IConnection> Connection::build(uint16_t queueSize) {
    return std::shared_ptr<IConnection>(Safe::new_obj<Connection>(queueSize));
}

Connection::Connection(uint16_t queueSize) 
    : nextMessage(queueSize),
      status(Status::Prepare) {}

Connection::~Connection() {
    this->client.stop();
}

Error::Code Connection::connect(char* ssid, char* pswd, char* host, uint16_t port) {
    Error::Code err = connectWifi(ssid, pswd);
    if (err != Error::Nil) {
        return err;
    }
    return connectHost(host, port);
}

Error::Code Connection::loopListen() {
    byte* buffer = Safe::new_arr<byte>(BUFFER_SIZE);
    if (buffer == nullptr) {
        return Error::NotEnoughMem;
    }

    bool header = true;
    byte* message = nullptr;
    uint16_t seek = 0;
    uint16_t readed = 0;
    uint16_t length = HEADER_SIZE;

    while (WiFi.status() == WL_CONNECTED && this->status == Status::Connected) {
        // "readed" always is <= "length - seek"
        readed = this->client.readBytes(buffer + seek, length - seek);
        if (readed <= 0) {
            continue;
        }

        // Read enough data
        seek += readed;
        if (seek < length) {
            continue;
        }

        // Read "data length"
        if (header) {
            length = (buffer[1]<<8) | buffer[0];
            if (length > 0) {
                header = false;
            }
            seek = 0;
            continue;
        }

        // Build "message"
        message = Safe::new_arr<byte>(length);
        if (message == nullptr) {
            delete[] buffer;
            return Error::NotEnoughMem;
        }

        // Push message
        // Don't delete "message" because std::share_ptr will manage
        memcpy(message, buffer, length);
        this->nextMessage.push(std::shared_ptr<byte>(message));

        // Reset
        length = HEADER_SIZE;
        header = true;
        seek = 0;
    }
    delete[] buffer;
    this->status = Status::Disconnected;
    return Error::NotConnectServer;
}

Error::Code Connection::sendMessage(byte* data, uint16_t nBytes) {
    if (this->status != Status::Connected) {
        return Error::NotConnectServer;
    }

    //============
    // Make buffer
    //============
    nBytes = nBytes + 2; // Add 2 bytes "data length"
    byte* buffer = Safe::new_arr<byte>(nBytes);
    if (buffer == nullptr) {
        return Error::NotEnoughMem;
    }
    memcpy(buffer, &nBytes, sizeof(uint16_t));
    memcpy(buffer + 2, data, nBytes - 2);

    //===============
    // Send and check
    //=============== 
    for (size_t seek = 0, sent = 0; seek < nBytes; seek += sent) {
        sent = this->client.write(buffer + seek, nBytes - seek);
        if (sent == 0) {
            this->status = Status::Disconnected;
            this->client.stop();
            delete[] buffer;
            return Error::NotConnectServer;
        }        
    }
    delete[] buffer;
    return Error::Nil;
}

std::shared_ptr<byte> Connection::getMessage() {
    return this->nextMessage.pop<byte>().second;
}

//========
// PRIVATE
//========
Error::Code Connection::connectWifi(char* ssid, char* pswd) {
    uint8_t retry = 0;
    WiFi.begin(ssid, pswd);
    while (WiFi.status() != WL_CONNECTED) {
        if (++retry >= 3) {
            return Error::NotConnectServer;
        }
        vTaskDelay(500);
    }
    return Error::Nil;
}

Error::Code Connection::connectHost(char* host, uint16_t port) {
    if (this->status == Status::Connected) {
        return Error::Nil;
    }
    uint8_t retry = 0;
    while (!client.connect(host, port)) {
        if (++retry >= 2) {
            return Error::NotConnectServer;
        }
        vTaskDelay(100);
    }
    this->status = Status::Connected;
    return Error::Nil;
}