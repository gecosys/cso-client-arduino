#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <memory>
#include <WiFi.h>
#include "status.h"
#include "utils/utils_buffer.h"

#define HEADER_SIZE 2
#define BUFFER_SIZE 1024

class Connection {
private:
    Buffer<byte*, POINTER_ARRAY> nextMessage;
    WiFiClient client;
    Status::Code status;

private:
    bool connectWifi(char* ssid, char* pswd);
    bool connectHost(char* host, uint16_t port);

public:
    Connection(uint16_t bufferSize);
    virtual ~Connection();

    bool connect(char* ssid, char* pswd, char* host, uint16_t port);
    void loopListen();
    bool sendMessage(byte* data, uint16_t nBytes);
    std::unique_ptr<byte> getMessage();
};

#endif //_CONNECTION_H_