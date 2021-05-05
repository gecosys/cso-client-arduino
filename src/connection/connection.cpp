#include "connection/connection.h"

Connection::Connection(uint16_t bufferSize) 
    : nextMessage(bufferSize),
      status(Status::Prepare) {}

Connection::~Connection() {
    this->client.stop();
}

bool Connection::connect(char* ssid, char* pswd, char* host, uint16_t port) {
    if (!connectWifi(ssid, pswd)) {
        return false;
    }
    return connectHost(host, port);
}

Error::Code Connection::loopListen() {
    bool readHeader = true;
    uint16_t seek = 0;
    uint16_t readed = 0;
    uint16_t lenBuffer = HEADER_SIZE;

    byte* buffer = (byte*)std::malloc(BUFFER_SIZE * sizeof(byte));
    if (!buffer) {
        return Error::NotEnoughMem;
    }

    while (WiFi.status() == WL_CONNECTED && this->status == Status::Connected) {
        readed = this->client.readBytes(buffer + seek, lenBuffer - seek);
        if (readed <= 0) {
            continue;
        }
        seek += readed;
        if (seek < lenBuffer) {
            continue;
        }

        // Solve "header"
        if (readHeader) {
            lenBuffer = (buffer[1]<<8) | buffer[0];
            if (lenBuffer > 0) {
                readHeader = false;
            }
            seek = 0;
            continue;
        }

        // Solve "body"
        this->nextMessage.push(std::shared_ptr<byte>(buffer));
        buffer = (byte*)std::malloc(BUFFER_SIZE * sizeof(byte));
        if (!buffer) {
            return Error::NotEnoughMem;
        }
        lenBuffer = HEADER_SIZE;
        readHeader = true;
        seek = 0;
    }
    this->status = Status::Disconnected;
    std::free(buffer);
    return Error::Disconnected;
}

bool Connection::sendMessage(byte* data, uint16_t nBytes) {
    if (this->status != Status::Connected) {
        return false;
    }

    //============
    // Make buffer
    //============
    nBytes = nBytes + 2; // Add 2 bytes "data length"
    byte* buffer = new byte[nBytes];
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
            return false;
        }        
    }
    delete[] buffer;
    return true;
}

std::shared_ptr<byte> Connection::getMessage() {
    std::shared_ptr<byte> ret(nullptr);
    this->nextMessage.pop<byte>(ret);
    return ret;
}

//========
// PRIVATE
//========
bool Connection::connectWifi(char* ssid, char* pswd) {
    uint8_t retry = 0;
    WiFi.begin(ssid, pswd);
    while (WiFi.status() != WL_CONNECTED) {
        if (++retry >= 3) {
            return false;
        }
        Serial.println("Connecting to Wifi...");
        vTaskDelay(500);
    }
    Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
}

bool Connection::connectHost(char* host, uint16_t port) {
    if (this->status == Status::Connected) {
        return true;
    }
    uint8_t retry = 0;
    while (!client.connect(host, port)) {
        if (++retry >= 2) {
            return false;
        }
        vTaskDelay(100);
    }
    this->status = Status::Connected;
    return true;
}