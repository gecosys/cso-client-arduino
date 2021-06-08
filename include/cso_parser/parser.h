#ifndef _CSO_PARSER_H_
#define _CSO_PARSER_H_

#include "interface.h"

class Parser : public IParser {
private:
    std::shared_ptr<byte> secretKey; // Const length is 32

public:
    static std::unique_ptr<IParser> build();

private:
    Parser() noexcept = default;

    MessageType getMessagetype(bool isGroup, bool isCached) noexcept;
    std::pair<Error::Code, Array<byte>> createMessage(uint64_t msgID, uint64_t msgTag, bool isGroup, const char* name, byte* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) noexcept;

public:
    Parser(Parser&& other) = delete;
    Parser(const Parser& other) = delete;
    Parser& operator=(const Parser& other) = delete;

    ~Parser() noexcept;

    void setSecretKey(std::shared_ptr<byte> secretKey) noexcept;
    std::pair<Error::Code, std::unique_ptr<Cipher>> parseReceivedMessage(byte* content, uint16_t lenContent) noexcept;
    std::pair<Error::Code, Array<byte>> buildActiveMessage(uint16_t ticketID, byte* ticketBytes, uint16_t lenTicket) noexcept;
    std::pair<Error::Code, Array<byte>> buildMessage(uint64_t msgID, uint64_t msgTag, const char* recvName, byte* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) noexcept;
    std::pair<Error::Code, Array<byte>> buildGroupMessage(uint64_t msgID, uint64_t msgTag, const char* groupName, byte* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) noexcept;
};

#endif //_CSO_PARSER_H_