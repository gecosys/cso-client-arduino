#ifndef _CSO_CONNECTOR_H_
#define _CSO_CONNECTOR_H_

#include <atomic>
#include "interface.h"
#include "config/config.h"
#include "cso_queue/item.h"
#include "cso_queue/interface.h"
#include "cso_proxy/interface.h"
#include "cso_parser/interface.h"
#include "cso_counter/interface.h"
#include "cso_connection/interface.h"

class Connector : public IConnector {
private:
    uint64_t time;
    std::atomic<bool> isActivated;
    std::atomic<bool> isDisconnected;
    ServerTicket serverTicket;
    std::unique_ptr<IProxy> proxy;
    std::unique_ptr<IParser> parser;
    std::shared_ptr<IConfig> config;
    std::unique_ptr<ICounter> counter;
    std::unique_ptr<IConnection> conn;
    std::unique_ptr<IQueue> queueMessages;

public:
    // inits a new instance of Connector interface with default values
    static std::unique_ptr<IConnector> build(int32_t bufferSize, std::shared_ptr<IConfig> config);

    // inits a new instance of Connector interface
    static std::unique_ptr<IConnector> build(int32_t bufferSize, std::unique_ptr<IQueue> queue, std::unique_ptr<IParser> parser, std::unique_ptr<IProxy> proxy, std::shared_ptr<IConfig> config);

private:
    Connector(
        int32_t bufferSize, 
        std::unique_ptr<IQueue>& queue,
        std::unique_ptr<IParser>& parser,
        std::unique_ptr<IProxy>& proxy,
        std::shared_ptr<IConfig>& config
    );

    Error::Code prepare();
    Error::Code activateConnection(uint16_t ticketID, uint8_t* ticketBytes, uint16_t lenTicket);
    Error::Code doSendMessageNotRetry(const char* name, uint8_t* content, uint16_t lenContent, bool isGroup,bool isEncrypted, bool isCache);
    Error::Code doSendMessageRetry(const char* recvName, uint8_t* content, uint16_t lenContent, bool isGroup,bool isEncrypted, int32_t retry);

public:
    Connector() = delete;
    Connector(Connector&& other) = delete;
    Connector(const Connector& other) = delete;
    Connector& operator=(const Connector& other) = delete;

    ~Connector() noexcept;

    void loopReconnect();
    void listen(Error::Code (*cb)(const char* sender, uint8_t* data, uint16_t lenData));

    Error::Code sendMessage(const char* recvName, uint8_t* content, uint16_t lenContent, bool isEncrypted, bool isCache);
    Error::Code sendGroupMessage(const char* groupName, uint8_t* content, uint16_t lenContent, bool isEncrypted, bool isCache);
    Error::Code sendMessageAndRetry(const char* recvName, uint8_t* content, uint16_t lenContent, bool isEncrypted, int32_t retry);
    Error::Code sendGroupMessageAndRetry(const char* groupName, uint8_t* content, uint16_t lenContent, bool isEncrypted, int32_t retry);
};

#endif //_CSO_CONNECTOR_H_