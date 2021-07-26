#ifndef CSO_PARSER_H
#define CSO_PARSER_H

#include "interface.h"

class Parser : public IParser {
private:
    Array<uint8_t> secretKey;

public:
    static std::unique_ptr<IParser> build();

private:
    Parser() noexcept = default;

    MessageType getMessagetype(bool isGroup, bool isCached) noexcept;

    std::tuple<Error::Code, Array<uint8_t>> createMessage(
        uint64_t msgID,
        uint64_t msgTag,
        bool isGroup,
        bool encrypted,
        bool cache,
        bool first,
        bool last,
        bool request,
        const std::string& name,
        const Array<uint8_t>& content);

public:
    Parser(Parser&& other) = delete;
    Parser(const Parser& other) = delete;
    ~Parser() noexcept = default;

    Parser& operator=(Parser&& other) = delete;
    Parser& operator=(const Parser& other) = delete;

    void setSecretKey(Array<uint8_t>&& secretKey) noexcept;
    void setSecretKey(const Array<uint8_t>& secretKey);

    std::tuple<Error::Code, std::unique_ptr<Cipher>> parseReceivedMessage(const Array<uint8_t>& content);
    std::tuple<Error::Code, Array<uint8_t>> buildActiveMessage(uint16_t ticketID, const Array<uint8_t>& ticketBytes);

    std::tuple<Error::Code, Array<uint8_t>> buildMessage(
        uint64_t msgID,
        uint64_t msgTag,
        bool encrypted,
        bool cache,
        bool first,
        bool last,
        bool request,
        const std::string& recvName,
        const Array<uint8_t>& content);

    std::tuple<Error::Code, Array<uint8_t>> buildGroupMessage(
        uint64_t msgID,
        uint64_t msgTag,
        bool encrypted,
        bool cache,
        bool first,
        bool last,
        bool request,
        const std::string& groupName,
        const Array<uint8_t>& content);
};

#endif // !CSO_PARSER_H