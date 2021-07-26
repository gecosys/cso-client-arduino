#ifndef CSO_CONNECTOR_H
#define CSO_CONNECTOR_H

#include <tuple>
#include <atomic>
#include "interface.h"
#include "cso_config/config.h"
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
    uint16_t ticketID;
    Array<uint8_t> ticketBytes;
    std::unique_ptr<IProxy> proxy;
    std::unique_ptr<IParser> parser;
    std::unique_ptr<ICounter> counter;
    std::unique_ptr<IConnection> connection;
    std::unique_ptr<IQueue> queueMessages;

public:
    // inits a new instance of Connector interface with default values
    static std::unique_ptr<IConnector> build(int32_t bufferSize, std::unique_ptr<IConfig>&& config);

    // inits a new instance of Connector interface
    static std::unique_ptr<IConnector> build(int32_t bufferSize, std::unique_ptr<IQueue>&& queue, std::unique_ptr<IParser>&& parser, std::unique_ptr<IProxy>&& proxy);

private:
    Connector(
        int32_t bufferSize, 
        std::unique_ptr<IQueue>&& queue,
        std::unique_ptr<IParser>&& parser,
        std::unique_ptr<IProxy>&& proxy
    );

    std::tuple<Error::Code, ServerTicket> prepare();
    Error::Code activateConnection();
    Error::Code doSendMessageNotRetry(const std::string& name, const Array<uint8_t>& content, bool isGroup, bool isEncrypted, bool isCache);
    Error::Code doSendMessageRetry(const std::string& recvName, const Array<uint8_t>& content, bool isGroup, bool isEncrypted, int32_t retry);

public:
    Connector() = delete;
    Connector(Connector&& other) = delete;
    Connector(const Connector& other) = delete;
    ~Connector() noexcept;

    Connector& operator=(Connector&& other) = delete;
    Connector& operator=(const Connector& other) = delete;

    void loopReconnect();
    void listen(Error::Code (*cb)(const std::string& sender, const Array<uint8_t>& data));

    Error::Code sendMessage(const std::string& recvName, const Array<uint8_t>& content, bool isEncrypted, bool isCache);
    Error::Code sendGroupMessage(const std::string& groupName, const Array<uint8_t>& content, bool isEncrypted, bool isCache);

    Error::Code sendMessageAndRetry(const std::string& recvName, const Array<uint8_t>& content, bool isEncrypted, int32_t retry);
    Error::Code sendGroupMessageAndRetry(const std::string& groupName, const Array<uint8_t>& content, bool isEncrypted, int32_t retry);
};

#endif // !CSO_CONNECTOR_H