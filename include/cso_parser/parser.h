#ifndef _CSO_PARSER_H_
#define _CSO_PARSER_H_

#include "interface.h"

class Parser : public IParser {
private:
    std::shared_ptr<uint8_t> secretKey; // Const length is 32

public:
    static std::unique_ptr<IParser> build();

private:
    Parser() noexcept = default;

    MessageType getMessagetype(bool isGroup, bool isCached) noexcept;
    Result<Array<uint8_t>> createMessage(uint64_t msgID, uint64_t msgTag, bool isGroup, const char* name, uint8_t* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) noexcept;

public:
    Parser(Parser&& other) = delete;
    Parser(const Parser& other) = delete;
    Parser& operator=(const Parser& other) = delete;

    ~Parser() noexcept;

    void setSecretKey(std::shared_ptr<uint8_t> secretKey) noexcept;
    Result<std::unique_ptr<Cipher>> parseReceivedMessage(uint8_t* content, uint16_t lenContent) noexcept;
    Result<Array<uint8_t>> buildActiveMessage(uint16_t ticketID, uint8_t* ticketBytes, uint16_t lenTicket) noexcept;
    Result<Array<uint8_t>> buildMessage(uint64_t msgID, uint64_t msgTag, const char* recvName, uint8_t* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) noexcept;
    Result<Array<uint8_t>> buildGroupMessage(uint64_t msgID, uint64_t msgTag, const char* groupName, uint8_t* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) noexcept;
};

#endif //_CSO_PARSER_H_