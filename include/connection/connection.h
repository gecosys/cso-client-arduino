#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <WiFi.h>
#include "status.h"
#include "interface.h"
#include "utils/utils_queue.h"

#define HEADER_SIZE 2
#define BUFFER_SIZE 1024

class Connection : public IConnection {
private:
    Queue<std::shared_ptr<byte>> nextMessage;
    WiFiClient client;
    Status::Code status;

public:
    static std::shared_ptr<IConnection> build(uint16_t queueSize);

private:
    Connection(uint16_t queueSize);
    Error::Code connectWifi(char* ssid, char* pswd);
    Error::Code connectHost(char* host, uint16_t port);

public:
    Connection() = delete;
    Connection(Connection&& other) = delete;
    Connection(const Connection& other) = delete;
    virtual ~Connection();

    Error::Code connect(char* ssid, char* pswd, char* host, uint16_t port);
    Error::Code loopListen();
    Error::Code sendMessage(byte* data, uint16_t nBytes);
    std::shared_ptr<byte> getMessage();
};

#endif //_CONNECTION_H_