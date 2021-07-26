#include <esp_log.h>
#include <esp_timer.h>
#include "cso_queue/queue.h"
#include "cso_proxy/proxy.h"
#include "cso_parser/parser.h"
#include "cso_counter/counter.h"
#include "cso_connector/connector.h"
#include "cso_connection/connection.h"
#include "message/readyticket.h"
#include "error/package/connector_err.h"

#define DELAY_TIME 3000
#define TIMESTAMP_SECS() esp_timer_get_time() / 1000000ULL
#define TIMESTAMP_MICRO_SECS() esp_timer_get_time()

// inits a new instance of Connector interface with default values
std::unique_ptr<IConnector> Connector::build(int32_t bufferSize, std::unique_ptr<IConfig>&& config) {
    return std::unique_ptr<IConnector>(new Connector(
        bufferSize, 
        Queue::build(bufferSize), 
        Parser::build(), 
        Proxy::build(std::forward<std::unique_ptr<IConfig>>(config))
    ));
}

// inits a new instance of Connector interface
std::unique_ptr<IConnector> Connector::build(int32_t bufferSize, std::unique_ptr<IQueue>&& queue, std::unique_ptr<IParser>&& parser, std::unique_ptr<IProxy>&& proxy) {
    return std::unique_ptr<IConnector>(new Connector(
        bufferSize, 
        std::forward<std::unique_ptr<IQueue>>(queue), 
        std::forward<std::unique_ptr<IParser>>(parser), 
        std::forward<std::unique_ptr<IProxy>>(proxy)
    ));
}

Connector::Connector(
    int32_t bufferSize, 
    std::unique_ptr<IQueue>&& queue,
    std::unique_ptr<IParser>&& parser,
    std::unique_ptr<IProxy>&& proxy
) : time{ 0 },
    isActivated{ false },
    isDisconnected{ true },
    ticketID{ 0 },
    ticketBytes{},
    proxy{ nullptr },
    parser{ nullptr },
    counter{ nullptr },
    connection{ Connection::build(bufferSize) },
    queueMessages{ nullptr } {
   this->proxy.swap(proxy);
   this->parser.swap(parser);
   this->queueMessages.swap(queue);
}

Connector::~Connector() noexcept {}

void Connector::loopReconnect() {
    Error::Code errcode;

    while (true) {
        // "WiFi" will auto reconnect
        if (WiFi.status() != WL_CONNECTED) {
            delay(DELAY_TIME / 3);
            continue;
        }

        // Do connect CSO system
        {
            // Exchange key
            ServerTicket ticket;
            std::tie(errcode, ticket) = prepare();
            if (errcode != Error::Code::Nil) {
                log_e("%s", Error::getString(errcode).c_str());
                delay(DELAY_TIME);
                continue;
            }

            // Connect to hub
            errcode = this->connection->connect(ticket.hubIP, ticket.hubPort);
            if (errcode != Error::Code::Nil) {
                log_e("%s", Error::getString(errcode).c_str());
                delay(DELAY_TIME);
                continue;
            }

            // Set values
            this->ticketID = ticket.ticketID;
            this->ticketBytes.swap(ticket.ticketBytes);
            this->parser->setSecretKey(std::move(ticket.serverSecretKey));
        }

        // Loop to receive message
        this->isDisconnected.store(false);
        errcode = this->connection->loopListen();
        if (errcode != Error::Code::Nil) {
            log_e("%s", Error::getString(errcode).c_str());
        }
        this->isActivated.store(false);
        this->isDisconnected.store(true);
    }
}

void Connector::listen(Error::Code (*cb)(const std::string& sender, const Array<uint8_t>& data)) {
    // Receive message response
    Array<uint8_t> cipher = this->connection->getMessage();
    if (!cipher.empty()) {
        Error::Code errcode;
        std::unique_ptr<Cipher> message;

        std::tie(errcode, message) = this->parser->parseReceivedMessage(cipher);
        if (errcode != Error::Code::Nil) {
            log_e("%s", Error::getString(errcode).c_str());
            return;
        }

        MessageType type = message->getMsgType();
        // Activate the connection
        if (type == MessageType::Activation) {
            std::unique_ptr<ReadyTicket> readyTicket;
            std::tie(errcode, readyTicket) = ReadyTicket::parseBytes(message->getData());
            if (errcode != Error::Code::Nil || !readyTicket->getIsReady()) {
                return;
            }

            this->isActivated.store(true, std::memory_order_release);
            if (this->counter == nullptr) {
                this->counter = Counter::build(readyTicket->getIdxWrite(), readyTicket->getIdxRead(), readyTicket->getMaskRead());
            }
            return;
        }

        if (!this->isActivated.load(std::memory_order_acquire)) {
            return;
        }

        if (type != MessageType::Done && 
            type != MessageType::Single &&
            type != MessageType::SingleCached &&
            type != MessageType::Group &&
            type != MessageType::GroupCached) {
            return;
        }

        if (message->getMsgID() == 0) {
            if (message->getIsRequest()) {
                cb(message->getName(), message->getData());
            }
            return;
        }

        if (!message->getIsRequest()) { //response
            this->queueMessages->clearMessage(message->getMsgID());
            return;
        }

        if (this->counter->markReadDone(message->getMsgTag())) {
            if (cb(message->getName(), message->getData()) != Error::Code::Nil) {
                this->counter->markReadUnused(message->getMsgTag());
                return;
            }
        }
        
        Array<uint8_t> newMessage;
        std::tie(errcode, newMessage) = this->parser->buildMessage(
            message->getMsgID(),
            message->getMsgTag(),
            message->getIsEncrypted(),
            false,
            true,
            true,
            false,
            message->getName(),
            Array<uint8_t>{}
        );
        if (errcode != Error::Code::Nil) {
            log_e("%s", Error::getString(errcode).c_str());
            return;
        }
        this->connection->sendMessage(newMessage);
    }

    if (this->isDisconnected.load(std::memory_order_acquire)) {
        return;
    }

    // Do activate the connection
    if (!this->isActivated.load(std::memory_order_acquire) && (TIMESTAMP_SECS() - this->time) >= 3) {
        Error::Code errcode = activateConnection();
        if (errcode != Error::Code::Nil) {
            log_e("%s", Error::getString(errcode).c_str());
        }
        this->time = TIMESTAMP_SECS();
        return;
    }

    // Send message in queue
    if (this->isActivated.load(std::memory_order_acquire) && (TIMESTAMP_MICRO_SECS() - this->time) >= 100) {
        ItemQueueRef ref_item = this->queueMessages->nextMessage();
        if (ref_item.empty()) {
            this->time = TIMESTAMP_MICRO_SECS();
            return;
        }

        Error::Code errcode;
        Array<uint8_t> message;
        ItemQueue& item = ref_item.get();

        if (item.isGroup) {
            std::tie(errcode, message) = this->parser->buildGroupMessage(
                item.msgID,
                item.msgTag,
                item.isEncrypted,
                item.isCached,
                item.isFirst,
                item.isLast,
                item.isRequest,
                item.recvName,
                item.content
            );
        }
        else {
            std::tie(errcode, message) = this->parser->buildMessage(
                item.msgID,
                item.msgTag,
                item.isEncrypted,
                item.isCached,
                item.isFirst,
                item.isLast,
                item.isRequest,
                item.recvName,
                item.content
            );
        }

        if (errcode != Error::Code::Nil) {
            this->time = TIMESTAMP_MICRO_SECS();
            log_e("%s", Error::getString(errcode).c_str());
            return;
        }

        errcode = this->connection->sendMessage(message);
        if (errcode != Error::Nil) {
            log_e("%s", Error::getString(errcode).c_str());
        }
        this->time = TIMESTAMP_MICRO_SECS();
    }
}

Error::Code Connector::sendMessage(const std::string& recvName, const Array<uint8_t>& content, bool isEncrypted, bool isCache) {
    if (!this->isActivated.load(std::memory_order_acquire)) {
        return Error::buildCode(
            ConnectorErr::ID,
            ConnectorErr::Func::Connector_SendMsg,
            ConnectorErr::Reason::Connection_NotActivatd
        );
    }
    return doSendMessageNotRetry(recvName, content, false, isEncrypted, isCache);
}

Error::Code Connector::sendGroupMessage(const std::string& groupName, const Array<uint8_t>& content, bool isEncrypted, bool isCache) {
    if (!this->isActivated.load(std::memory_order_acquire)) {
        return Error::buildCode(
            ConnectorErr::ID,
            ConnectorErr::Func::Connector_SendGroupMsg,
            ConnectorErr::Reason::Connection_NotActivatd
        );
    }
    return doSendMessageNotRetry(groupName, content, true, isEncrypted, isCache);
}

Error::Code Connector::sendMessageAndRetry(const std::string& recvName, const Array<uint8_t>& content, bool isEncrypted, int32_t retry) {
    if (!this->isActivated.load(std::memory_order_acquire)) {
        return Error::buildCode(
            ConnectorErr::ID,
            ConnectorErr::Func::Connector_SendMsgRetry,
            ConnectorErr::Reason::Connection_NotActivatd
        );
    }

    if (!this->queueMessages->takeIndex()) {
        return Error::buildCode(
            ConnectorErr::ID,
            ConnectorErr::Func::Connector_SendMsgRetry,
            ConnectorErr::Reason::MsgQueue_Full
        );
    }

    return doSendMessageRetry(recvName, content, false, isEncrypted, retry);
}

Error::Code Connector::sendGroupMessageAndRetry(const std::string& groupName, const Array<uint8_t>& content, bool isEncrypted, int32_t retry) {
    if (!this->isActivated.load(std::memory_order_acquire)) {
        return Error::buildCode(
            ConnectorErr::ID,
            ConnectorErr::Func::Connector_SendGroupMsgRetry,
            ConnectorErr::Reason::Connection_NotActivatd
        );
    }

    if (!this->queueMessages->takeIndex()) {
        return Error::buildCode(
            ConnectorErr::ID,
            ConnectorErr::Func::Connector_SendGroupMsgRetry,
            ConnectorErr::Reason::MsgQueue_Full
        );
    }
    return doSendMessageRetry(groupName, content, true, isEncrypted, retry);
}

//========
// PRIVATE
//========
std::tuple<Error::Code, ServerTicket> Connector::prepare() {
    Error::Code errcode;
    ServerKey serverKey;

    std::tie(errcode, serverKey) = this->proxy->exchangeKey();
    if (errcode != Error::Code::Nil) {
        return std::make_tuple(errcode, ServerTicket{});
    }
    return this->proxy->registerConnection(serverKey);
}

Error::Code Connector::activateConnection() {
    Error::Code errcode;
    Array<uint8_t> message;

    std::tie(errcode, message) = this->parser->buildActiveMessage(this->ticketID, this->ticketBytes);
    if (errcode != Error::Code::Nil) {
        return errcode;
    }    
    return this->connection->sendMessage(message);
}

Error::Code Connector::doSendMessageNotRetry(const std::string& name, const Array<uint8_t>& content, bool isGroup, bool isEncrypted, bool isCache) {
    Error::Code errcode;
    Array<uint8_t> message;

    if (!isGroup) {
        std::tie(errcode, message) = this->parser->buildMessage(0, 0, isEncrypted, isCache, true, true, true, name, content);
    }
    else {
        std::tie(errcode, message) = this->parser->buildGroupMessage(0, 0, isEncrypted, isCache, true, true, true, name, content);
    }
    if (errcode != Error::Code::Nil) {
        return errcode;
    }
    return this->connection->sendMessage(message);
}

Error::Code Connector::doSendMessageRetry(const std::string& recvName, const Array<uint8_t>& content, bool isGroup, bool isEncrypted, int32_t retry) {
    std::unique_ptr<ItemQueue> item{ new ItemQueue{} };
    item->content = content;
    item->msgID = this->counter->nextWriteIndex();
    item->msgTag = 0;
    item->recvName = recvName;
    item->isEncrypted = isEncrypted;
    item->isCached = false;
    item->isFirst = true;
    item->isLast = true;
    item->isRequest = true;
    item->isGroup = isGroup;
    item->numberRetry = retry + 1;
    item->timestamp = 0;
    this->queueMessages->pushMessage(std::move(item));
    return Error::Code::Nil;
}