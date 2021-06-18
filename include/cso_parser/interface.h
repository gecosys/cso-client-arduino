#ifndef _CSO_PARSER_INTERFACE_H_
#define _CSO_PARSER_INTERFACE_H_

#include "message/cipher.h"
#include "error/error_code.h"

class IParser {
public:
    virtual void setSecretKey(std::shared_ptr<uint8_t> secretKey) noexcept = 0;
    virtual Result<std::unique_ptr<Cipher>> parseReceivedMessage(uint8_t* content, uint16_t lenContent) = 0;
    virtual Result<Array<uint8_t>> buildActiveMessage(uint16_t ticketID, uint8_t* ticketBytes, uint16_t lenTicket) = 0;
    virtual Result<Array<uint8_t>> buildMessage(uint64_t msgID, uint64_t msgTag, const char* recvName, uint8_t* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) = 0;
    virtual Result<Array<uint8_t>> buildGroupMessage(uint64_t msgID, uint64_t msgTag, const char* groupName, uint8_t* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) = 0;
};

#endif //_CSO_PARSER_INTERFACE_H_