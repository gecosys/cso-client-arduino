#ifndef _CSO_CONNECTOR_H_
#define _CSO_CONNECTOR_H_

#include "interface.h"
#include "config/config.h"
#include "utils/utils_safe.h"
#include "cso_queue/item.h"
#include "cso_queue/interface.h"
#include "cso_proxy/interface.h"
#include "cso_parser/interface.h"
#include "cso_counter/interface.h"
#include "cso_connection/interface.h"

class Connector : public IConnector {
private:
    uint64_t time;
    bool isActivated;
    bool isDisconnected;
    ServerTicket serverTicket;
    std::shared_ptr<IProxy> proxy;
    std::shared_ptr<IParser> parser;
    std::shared_ptr<IConfig> config;
    std::shared_ptr<ICounter> counter;
    std::shared_ptr<IConnection> conn;
    std::shared_ptr<IQueue> queueMessages;

public:
    // inits a new instance of Connector interface with default values
    static std::shared_ptr<IConnector> build(int32_t bufferSize, std::shared_ptr<IConfig> config);

    // inits a new instance of Connector interface
    static std::shared_ptr<IConnector> build(
        int32_t bufferSize, 
        std::shared_ptr<IQueue> queue,
        std::shared_ptr<IParser> parser,
        std::shared_ptr<IProxy> proxy,
        std::shared_ptr<IConfig> config
    );

private:
    friend class Safe;
    Connector() = default;
    Connector(
        int32_t bufferSize, 
        std::shared_ptr<IQueue> queue,
        std::shared_ptr<IParser> parser,
        std::shared_ptr<IProxy> proxy,
        std::shared_ptr<IConfig> config
    );

    Error::Code prepare();
    Error::Code activateConnection(uint16_t ticketID, byte* ticketBytes, uint16_t lenTicket);
    Error::Code doSendResponse(
        uint64_t msgID, 
        uint64_t msgTag, 
        const char* recvName, 
        byte* data, 
        uint16_t lenData,
        bool isEncrypted
    );
    Error::Code doSendMessageNotRetry(
        const char* name, 
        byte* content, 
        uint16_t lenContent, 
        bool isGroup,
        bool isEncrypted, 
        bool isCache
    );
    Error::Code doSendMessageRetry(
        const char* recvName, 
        byte* content, 
        uint16_t lenContent, 
        bool isGroup,
        bool isEncrypted, 
        int32_t retry
    );

public:
    Connector(Connector&& other) = delete;
    Connector(const Connector& other) = delete;
    virtual ~Connector() noexcept;

    void loopReconnect();
    void listen(Error::Code (*cb)(const char* sender, byte* data, uint16_t lenData));

    Error::Code sendMessage(
        const char* recvName, 
        byte* content, 
        uint16_t lenContent, 
        bool isEncrypted, 
        bool isCache
    );
    Error::Code sendGroupMessage(
        const char* groupName, 
        byte* content, 
        uint16_t lenContent, 
        bool isEncrypted, 
        bool isCache
    );

    Error::Code sendMessageAndRetry(
        const char* recvName, 
        byte* content, 
        uint16_t lenContent, 
        bool isEncrypted, 
        int32_t retry
    );
    Error::Code sendGroupMessageAndRetry(
        const char* groupName, 
        byte* content, 
        uint16_t lenContent, 
        bool isEncrypted, 
        int32_t retry
    );
};

#endif //_CSO_CONNECTOR_H_