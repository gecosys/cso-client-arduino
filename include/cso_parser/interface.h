#ifndef CSO_PARSER_INTERFACE_H
#define CSO_PARSER_INTERFACE_H

#include <tuple>
#include "error/error.h"
#include "entity/array.h"
#include "message/cipher.h"

class IParser {
public:
    virtual void setSecretKey(Array<uint8_t>&& secretKey) noexcept = 0;
    virtual void setSecretKey(const Array<uint8_t>& secretKey) = 0;
    virtual std::tuple<Error::Code, std::unique_ptr<Cipher>> parseReceivedMessage(const Array<uint8_t>& content) = 0;
    virtual std::tuple<Error::Code, Array<uint8_t>> buildActiveMessage(uint16_t ticketID, const Array<uint8_t>& ticketBytes) = 0;
    virtual std::tuple<Error::Code, Array<uint8_t>> buildMessage(uint64_t msgID, uint64_t msgTag, bool encrypted, bool cache, bool first, bool last, bool request, const std::string& recvName, const Array<uint8_t>& content) = 0;
    virtual std::tuple<Error::Code, Array<uint8_t>> buildGroupMessage(uint64_t msgID, uint64_t msgTag, bool encrypted, bool cache, bool first, bool last, bool request, const std::string& groupName, const Array<uint8_t>& content) = 0;
};

#endif // !CSO_PARSER_INTERFACE_H