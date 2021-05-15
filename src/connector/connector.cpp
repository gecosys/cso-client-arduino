#include <esp_log.h>
#include <esp_timer.h>
#include "cso_queue/queue.h"
#include "cso_proxy/proxy.h"
#include "cso_parser/parser.h"
#include "cso_counter/counter.h"
#include "cso_connector/connector.h"
#include "cso_connection/connection.h"
#include "message/readyticket.h"

#define CSO_PORT 80
#define DELAY_TIME 3000
#define TIMESTAMP_SECS() esp_timer_get_time() / 1000000ULL
#define TIMESTAMP_MICRO_SECS() esp_timer_get_time()

// inits a new instance of Connector interface with default values
std::shared_ptr<IConnector> Connector::build(int32_t bufferSize, std::shared_ptr<IConfig> config) {
    IConnector* obj = Safe::new_obj<Connector>(
        bufferSize,
        Queue::build(bufferSize),
        Parser::build(),
        Proxy::build(config),
        config
    );
    if (obj == nullptr) {
        throw std::runtime_error("[cso_connector/Connector::build()]Not enough memory to create object");
    }
    return std::shared_ptr<IConnector>(obj);
}

// inits a new instance of Connector interface
std::shared_ptr<IConnector> Connector::build(
    int32_t bufferSize, 
    std::shared_ptr<IQueue> queue,
    std::shared_ptr<IParser> parser,
    std::shared_ptr<IProxy> proxy,
    std::shared_ptr<IConfig> config
) {
    IConnector* obj = Safe::new_obj<Connector>(bufferSize, queue, parser, proxy, config);
    if (obj == nullptr) {
        throw "[cso_connector/Connector::build()]Not enough memory to create object";
    }
    return std::shared_ptr<IConnector>(obj);
}

Connector::Connector(
    int32_t bufferSize, 
    std::shared_ptr<IQueue> queue,
    std::shared_ptr<IParser> parser,
    std::shared_ptr<IProxy> proxy,
    std::shared_ptr<IConfig> config
) : time(0),
    isActivated(false),
    isDisconnected(false),
    serverTicket(),
    proxy(proxy),
    parser(parser),
    config(config),
    counter(nullptr),
    conn(Connection::build(bufferSize)),
    queueMessages(Queue::build(bufferSize)) {}

Connector::~Connector() {}

void Connector::loopReconnect() {
    Error::Code error;
    while (true) {
        // "WiFi" will auto reconnect
        if (WiFi.status() != WL_CONNECTED) {
            vTaskDelay(DELAY_TIME / 3);
            continue;
        }

        error = prepare();
        if (error != Error::Nil) {
            log_e("Prepare failed with error code: %d", error);
            vTaskDelay(DELAY_TIME);
            continue;
        }

        // Connect to Clound Socket system
        this->parser->setSecretKey(this->serverTicket.serverSecretKey);
        error = this->conn->connect( 
            this->serverTicket.hubAddress.c_str(), 
            CSO_PORT
        );
        if (error != Error::Nil) {
            log_e("Connect failed with error code: %d", error);
            vTaskDelay(DELAY_TIME);
            continue;
        }

        // Loop to receive message
        this->isActivated = false;
        this->isDisconnected = false;
        error = this->conn->loopListen();
        if (error != Error::Nil) {
            log_e("Listen failed with error code: %d", error);
        }
        this->isDisconnected = true;
        vTaskDelay(DELAY_TIME);
    }
}

void Connector::listen(Error::Code (*cb)(const char* sender, byte* data, uint16_t lenData)) {
    // Receive message response
    Array<byte> content = this->conn->getMessage();
    if (content.ptr.get() != nullptr) {
        auto msg = this->parser->parseReceivedMessage(content.ptr.get(), content.length);
        if (msg.first != Error::Nil) {
            return;
        }

        MessageType type = msg.second->getMsgType();
        // Activate the connection
        if (type == MessageType::Activation) {
            auto readyTicket = ReadyTicket::parseBytes(msg.second->getData(), LENGTH_OUTPUT);
            if (readyTicket.errorCode != SUCCESS || !readyTicket.data->getIsReady()) {
                return;
            }
            this->isActivated = true;
            if (this->counter.get() == nullptr) {
                this->counter.reset(Safe::new_obj<Counter>(
                    readyTicket.data->getIdxWrite(),
                    readyTicket.data->getIdxRead(),
                    readyTicket.data->getMaskRead()
                ));
                if (this->counter.get() == nullptr) {
                    log_e("Not enough memory to create object");
                }
            }
            return;
        }

        if (!this->isActivated) {
            return;
        }

        if (type != MessageType::Done && 
            type != MessageType::Single &&
            type != MessageType::SingleCached &&
            type != MessageType::Group &&
            type != MessageType::GroupCached) {
            return;
        }

        if (msg.second->getMsgID() == 0) {
            if (msg.second->getIsRequest()) {
                cb(msg.second->getName(), msg.second->getData(), msg.second->getSizeData());
            }
            return;
        }

        if (!msg.second->getIsRequest()) { //response
            this->queueMessages->clearMessage(msg.second->getMsgID());
            return;
        }

        if (this->counter->markReadDone(msg.second->getMsgTag())) {
            if (cb(msg.second->getName(), msg.second->getData(), msg.second->getSizeData()) != Error::Nil) {
                this->counter->markReadUnused(msg.second->getMsgTag());
                return;
            }
        }
        doSendResponse(
            msg.second->getMsgID(), 
            msg.second->getMsgTag(),
            msg.second->getName(),
            nullptr,
            0,
            msg.second->getIsEncrypted()
        );
    }

    // Do activate the connection
    if (!this->isDisconnected && !this->isActivated && (TIMESTAMP_SECS() - this->time) >= 3) {
        Error::Code error = activateConnection(
            this->serverTicket.ticketID,
            this->serverTicket.ticketBytes.get(),
            LENGTH_TICKET
        );
        if (error != Error::Nil) {
            log_e("Activate failed with error code: %d", error);
        }
        vTaskDelay(100);
        this->time = TIMESTAMP_SECS();
        return;
    }

    if (TIMESTAMP_MICRO_SECS() - this->time >= 100) {
        auto msg = this->queueMessages->nextMessage();
        if (msg == nullptr) {
            this->time = TIMESTAMP_MICRO_SECS();
            return;
        }

        auto data = msg->get();
        std::pair<Error::Code, Array<byte>> content;
        if (data->isGroup) {
            content = this->parser->buildGroupMessage(
                data->msgID,
                data->msgTag,
                data->recvName.c_str(),
                data->content.ptr.get(),
                data->content.length,
                data->isEncrypted,
                data->isCached,
                data->isFirst,
                data->isLast,
                data->isRequest
            );
        } else {
            content = this->parser->buildMessage(
                data->msgID,
                data->msgTag,
                data->recvName.c_str(),
                data->content.ptr.get(),
                data->content.length,
                data->isEncrypted,
                data->isCached,
                data->isFirst,
                data->isLast,
                data->isRequest
            );
        }
        if (content.first != Error::Nil) {
            this->time = TIMESTAMP_MICRO_SECS();
            return;
        }
        this->conn->sendMessage(content.second.ptr.get(), content.second.length);
        this->time = TIMESTAMP_MICRO_SECS();
    }
}

Error::Code Connector::sendMessage(const char* recvName, byte* content, uint16_t lenContent, bool isEncrypted, bool isCache) {
    return doSendMessageNotRetry(recvName, content, lenContent, false, isEncrypted, isCache);
}

Error::Code Connector::sendGroupMessage(const char* groupName, byte* content, uint16_t lenContent, bool isEncrypted, bool isCache) {
    return doSendMessageNotRetry(groupName, content, lenContent, true, isEncrypted, isCache);
}

Error::Code Connector::sendMessageAndRetry(const char* recvName, byte* content, uint16_t lenContent, bool isEncrypted, int32_t retry) {
    return doSendMessageRetry(recvName, content, lenContent, false, isEncrypted, retry);
}

Error::Code Connector::sendGroupMessageAndRetry(const char* groupName, byte* content, uint16_t lenContent, bool isEncrypted, int32_t retry) {
    return doSendMessageRetry(groupName, content, lenContent, true, isEncrypted, retry);
}

//========
// PRIVATE
//========
Error::Code Connector::prepare() {
    auto respExchangeKey = this->proxy->exchangeKey();
    if (respExchangeKey.first != Error::Nil) {
        return respExchangeKey.first;
    }
    auto respRegConn =  this->proxy->registerConnection(respExchangeKey.second);
    if (respRegConn.first != Error::Nil) {
        return respRegConn.first;
    }
    this->serverTicket = respRegConn.second;
    return Error::Nil;
}

Error::Code Connector::activateConnection(uint16_t ticketID, byte* ticketBytes, uint16_t lenTicket) {
    auto res = this->parser->buildActiveMessage(ticketID, ticketBytes, lenTicket);
    if (res.first != Error::Nil) {
        return res.first;
    }
    return this->conn->sendMessage(res.second.ptr.get(), res.second.length);
}

Error::Code Connector::doSendResponse(uint64_t msgID, uint64_t msgTag, const char* recvName, byte* data, uint16_t lenData, bool isEncrypted) {
    auto msg = this->parser->buildMessage(
        msgID, 
        msgTag, 
        recvName, 
        data, 
        lenData, 
        isEncrypted, 
        false, 
        true, 
        true, 
        false
    );
    if (msg.first != Error::Nil) {
        return msg.first;
    }
    return this->conn->sendMessage(msg.second.ptr.get(), msg.second.length);
}

Error::Code Connector::doSendMessageNotRetry(const char* name, byte* content, uint16_t lenContent, bool isGroup, bool isEncrypted, bool isCache) {
    if (!this->isActivated) {
        return Error::NotReady;
    }
    std::pair<Error::Code, Array<byte>> data;
    if (!isGroup) {
        data = this->parser->buildMessage(
            0, 0, 
            name, 
            content, 
            lenContent, 
            isEncrypted, 
            isCache, 
            true, true, true
        );
    } else {
        data = this->parser->buildGroupMessage(
            0, 0, 
            name, 
            content, 
            lenContent, 
            isEncrypted, 
            isCache, 
            true, true, true
        );
    }
    if (data.first != Error::Nil) {
        return data.first;
    }
    return this->conn->sendMessage(data.second.ptr.get(), data.second.length);
}

Error::Code Connector::doSendMessageRetry(const char* name, byte* content, uint16_t lenContent, bool isGroup, bool isEncrypted, int32_t retry) {
    if (!this->isActivated) {
		return Error::NotReady;
	}

	if (!this->queueMessages->takeIndex()) {
		return Error::Full;
	}

	this->queueMessages->pushMessage(std::shared_ptr<ItemQueue>(Safe::new_obj<ItemQueue>(
        this->counter->nextWriteIndex(),
        0,
        name,
        content,
        lenContent,
        isEncrypted,
        false,
        true,
        true,
        true,
        isGroup,
        retry + 1,
        0
    )));
	return Error::Nil;
}