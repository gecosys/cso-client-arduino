#ifndef _CSO_PARSER_INTERFACE_H_
#define _CSO_PARSER_INTERFACE_H_

#include <memory>
#include <utility>
#include <Arduino.h>
#include "message/cipher.h"
#include "error/error_code.h"

class IParser {
public:
    virtual void setSecretKey(std::shared_ptr<byte> secretKey) noexcept = 0;
    virtual std::pair<Error::Code, std::unique_ptr<Cipher>> parseReceivedMessage(byte* content, uint16_t lenContent) = 0;
    virtual std::pair<Error::Code, Array<byte>> buildActiveMessage(uint16_t ticketID, byte* ticketBytes, uint16_t lenTicket) = 0;
    virtual std::pair<Error::Code, Array<byte>> buildMessage(uint64_t msgID, uint64_t msgTag, const char* recvName, byte* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) = 0;
    virtual std::pair<Error::Code, Array<byte>> buildGroupMessage(uint64_t msgID, uint64_t msgTag, const char* groupName, byte* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) = 0;
};

#endif //_CSO_PARSER_INTERFACE_H_