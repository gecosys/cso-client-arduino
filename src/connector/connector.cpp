#include <esp_log.h>
#include <esp_timer.h>
#include "cso_queue/queue.h"
#include "cso_proxy/proxy.h"
#include "cso_parser/parser.h"
#include "cso_counter/counter.h"
#include "cso_connector/connector.h"
#include "cso_connection/connection.h"
#include "message/readyticket.h"

#define DELAY_TIME 3000
#define TIMESTAMP_SECS() esp_timer_get_time() / 1000000ULL
#define TIMESTAMP_MICRO_SECS() esp_timer_get_time()

// inits a new instance of Connector interface with default values
std::unique_ptr<IConnector> Connector::build(int32_t bufferSize, std::shared_ptr<IConfig> config) {
    auto queue = Queue::build(bufferSize);
    auto parser = Parser::build();
    auto proxy = Proxy::build(config);
    IConnector* obj = new (std::nothrow) Connector(bufferSize, queue, parser, proxy,config);
    if (obj == nullptr) {
        throw std::runtime_error("[cso_connector/Connector::build(...)]Not enough memory to create object");
    }
    return std::unique_ptr<IConnector>(obj);
}

// inits a new instance of Connector interface
std::unique_ptr<IConnector> Connector::build(int32_t bufferSize, std::unique_ptr<IQueue> queue, std::unique_ptr<IParser> parser, std::unique_ptr<IProxy> proxy, std::shared_ptr<IConfig> config) {
    IConnector* obj = new (std::nothrow) Connector(bufferSize, queue, parser, proxy, config);
    if (obj == nullptr) {
        throw "[cso_connector/Connector::build(...)]Not enough memory to create object";
    }
    return std::unique_ptr<IConnector>(obj);
}

Connector::Connector(
    int32_t bufferSize, 
    std::unique_ptr<IQueue>& queue,
    std::unique_ptr<IParser>& parser,
    std::unique_ptr<IProxy>& proxy,
    std::shared_ptr<IConfig>& config
) : time(0),
    isActivated(false),
    isDisconnected(true),
    serverTicket(),
    proxy(nullptr),
    parser(nullptr),
    config(config),
    counter(nullptr),
    conn(Connection::build(bufferSize)),
    queueMessages(nullptr) {
   this->proxy.swap(proxy);
   this->parser.swap(parser);
   this->queueMessages.swap(queue);
}

Connector::~Connector() noexcept {}

void Connector::loopReconnect() {
    Error::Code error;
    while (true) {
        // "WiFi" will auto reconnect
        if (WiFi.status() != WL_CONNECTED) {
            vTaskDelay((DELAY_TIME / 3) / portTICK_PERIOD_MS);
            continue;
        }

        error = prepare();
        if (error != Error::Nil) {
            log_e("%s", Error::getContent(error));
            vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
            continue;
        }

        // Connect to Clound Socket system
        this->parser->setSecretKey(this->serverTicket.serverSecretKey);
        error = this->conn->connect( 
            this->serverTicket.hubAddress.ip(), 
            this->serverTicket.hubAddress.port()
        );
        if (error != Error::Nil) {
            log_e("%s", Error::getContent(error));
            vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
            continue;
        }

        // Loop to receive message
        this->isDisconnected.store(false);
        error = this->conn->loopListen();
        if (error != Error::Nil) {
            log_e("%s", Error::getContent(error));
            if (error == Error::CSOConnection_Disconnected) {
                this->isActivated.store(false);
                this->isDisconnected.store(true);
            }
        }
    }
}

void Connector::listen(Error::Code (*cb)(const char* sender, byte* data, uint16_t lenData)) {
    // Receive message response
    Array<byte> cipher_msg = this->conn->getMessage();
    if (cipher_msg.buffer != nullptr) {
        auto msg = this->parser->parseReceivedMessage(cipher_msg.buffer.get(), cipher_msg.length);
        if (msg.first != Error::Nil) {
            log_e("%s", Error::getContent(msg.first));
            return;
        }

        MessageType type = msg.second->getMsgType();
        // Activate the connection
        if (type == MessageType::Activation) {
            auto readyTicket = ReadyTicket::parseBytes(msg.second->getData(), msg.second->getSizeData());
            if (readyTicket.errorCode != Error::Nil || !readyTicket.data->getIsReady()) {
                return;
            }
            this->isActivated.store(true);
            if (this->counter == nullptr) {
                this->counter = Counter::build(readyTicket.data->getIdxWrite(), readyTicket.data->getIdxRead(), readyTicket.data->getMaskRead());
                if (this->counter == nullptr) {
                    log_e("[CSO_Connector]Not enough memory to create object");
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
        
        auto new_msg = this->parser->buildMessage(
            msg.second->getMsgID(), 
            msg.second->getMsgTag(),
            msg.second->getName(),
            nullptr,
            0,
            msg.second->getIsEncrypted(), 
            false, 
            true, 
            true, 
            false
        );
        if (new_msg.first != Error::Nil) {
            log_e("%s", Error::getContent(msg.first));
            return;
        }
        this->conn->sendMessage(new_msg.second.buffer.get(), new_msg.second.length);
    }

    if (this->isDisconnected.load()) {
        return;
    }

    // Do activate the connection
    if (!this->isActivated.load() && (TIMESTAMP_SECS() - this->time) >= 3) {
        Error::Code error = activateConnection(
            this->serverTicket.ticketID,
            this->serverTicket.ticketBytes.get(),
            LENGTH_TICKET
        );
        if (error != Error::Nil) {
            log_e("%s", Error::getContent(error));
        }
        this->time = TIMESTAMP_SECS();
        return;
    }

    // Send message in queue
    if (this->isActivated.load() && (TIMESTAMP_MICRO_SECS() - this->time) >= 100) {
        ItemQueueRef ref_msg = this->queueMessages->nextMessage();
        if (ref_msg.empty()) {
            this->time = TIMESTAMP_MICRO_SECS();
            return;
        }

        ItemQueue msg = ref_msg.get();
        std::pair<Error::Code, Array<byte>> content;
        if (msg.isGroup) {
            content = this->parser->buildGroupMessage(
                msg.msgID,
                msg.msgTag,
                msg.recvName.c_str(),
                msg.content.buffer.get(),
                msg.content.length,
                msg.isEncrypted,
                msg.isCached,
                msg.isFirst,
                msg.isLast,
                msg.isRequest
            );
        } else {
            content = this->parser->buildMessage(
                msg.msgID,
                msg.msgTag,
                msg.recvName.c_str(),
                msg.content.buffer.get(),
                msg.content.length,
                msg.isEncrypted,
                msg.isCached,
                msg.isFirst,
                msg.isLast,
                msg.isRequest
            );
        }
        if (content.first != Error::Nil) {
            this->time = TIMESTAMP_MICRO_SECS();
            return;
        }
        this->conn->sendMessage(content.second.buffer.get(), content.second.length);
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
    auto msg = this->parser->buildActiveMessage(ticketID, ticketBytes, lenTicket);
    if (msg.first != Error::Nil) {
        return msg.first;
    }    
    return this->conn->sendMessage(msg.second.buffer.get(), msg.second.length);
}

Error::Code Connector::doSendMessageNotRetry(const char* name, byte* content, uint16_t lenContent, bool isGroup, bool isEncrypted, bool isCache) {
    if (!this->isActivated.load()) {
        return Error::CSOConnector_NotActivated;
    }
    std::pair<Error::Code, Array<byte>> data;
    if (!isGroup) {
        data = this->parser->buildMessage(
            0, 
            0, 
            name, 
            content, 
            lenContent, 
            isEncrypted, 
            isCache, 
            true, 
            true, 
            true
        );
    } else {
        data = this->parser->buildGroupMessage(
            0, 
            0, 
            name, 
            content, 
            lenContent, 
            isEncrypted, 
            isCache, 
            true, 
            true, 
            true
        );
    }
    if (data.first != Error::Nil) {
        return data.first;
    }
    return this->conn->sendMessage(data.second.buffer.get(), data.second.length);
}

Error::Code Connector::doSendMessageRetry(const char* name, byte* content, uint16_t lenContent, bool isGroup, bool isEncrypted, int32_t retry) {
    if (!this->isActivated.load()) {
		return Error::CSOConnector_NotActivated;
	}

	if (!this->queueMessages->takeIndex()) {
		return Error::CSOConnector_MessageQueueFull;
	}

	this->queueMessages->pushMessage(new (std::nothrow) ItemQueue(
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
    ));
	return Error::Nil;
}