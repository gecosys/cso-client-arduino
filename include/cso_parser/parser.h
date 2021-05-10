#ifndef _CSO_PARSER_H_
#define _CSO_PARSER_H_

#include "interface.h"
#include "utils/utils_safe.h"

class Parser : public IParser {
private:
    std::shared_ptr<byte> secretKey; // Const length is 32

public:
    static std::shared_ptr<IParser> build();

private:
    Parser() = default;

    MessageType getMessagetype(bool isGroup, bool isCached);
    std::pair<Error::Code, Array<byte>> createMessage(
        uint64_t msgID, 
        uint64_t msgTag,
        bool isGroup,
        const char* name,
        byte* content,
        uint16_t lenContent,
        bool encrypted,
        bool cache,
        bool first,
        bool last,
        bool request
    );

public:
    friend class Safe;
    Parser(Parser&& other) = delete;
    Parser(const Parser& other) = delete;
    virtual ~Parser() noexcept;

    void setSecretKey(std::shared_ptr<byte> secretKey) noexcept;
    std::pair<Error::Code, std::shared_ptr<Cipher>> parseReceivedMessage(byte* content, uint16_t lenContent);
    
    std::pair<Error::Code, Array<byte>> buildActiveMessage(
        uint16_t ticketID, 
        byte* ticketBytes, 
        uint16_t lenTicket
    );
    
    std::pair<Error::Code, Array<byte>> buildMessage(
        uint64_t msgID, 
        uint64_t msgTag,
        const char* recvName,
        byte* content,
        uint16_t lenContent,
        bool encrypted,
        bool cache,
        bool first,
        bool last,
        bool request
    );

    std::pair<Error::Code, Array<byte>> buildGroupMessage(
        uint64_t msgID, 
        uint64_t msgTag,
        const char* groupName,
        byte* content,
        uint16_t lenContent,
        bool encrypted,
        bool cache,
        bool first,
        bool last,
        bool request
    );
};

#endif //_CSO_PARSER_H_