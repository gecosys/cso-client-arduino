#include <WString.h>
#include "cso_parser/parser.h"
#include "utils/utils_aes.h"
#include "utils/utils_hmac.h"

std::unique_ptr<IParser> Parser::build() {
    return std::unique_ptr<IParser>(new Parser());
}

Parser::~Parser() noexcept {}

void Parser::setSecretKey(std::shared_ptr<uint8_t> secretKey) noexcept {
    this->secretKey.swap(secretKey);
}

Result<std::unique_ptr<Cipher>> Parser::parseReceivedMessage(uint8_t* content, uint16_t lenContent) noexcept {
    // Parse message
    Result<std::unique_ptr<Cipher>> msg = Cipher::parseBytes(content, lenContent);
    if (msg.errorCode != Error::Nil) {
        return msg;
    }

    // Solve if message is not encrypted
    if (!msg.data->getIsEncrypted()) {
        Result<Array<uint8_t>> rawBytes = msg.data->getRawBytes();
        if (rawBytes.errorCode != Error::Nil) {
            msg.errorCode = rawBytes.errorCode;
            msg.data.reset(nullptr);
            return msg;
        }

        if (!UtilsHMAC::validateHMAC(
            this->secretKey.get(), 
            rawBytes.data.buffer.get(), 
            rawBytes.data.length, 
            msg.data->getSign()
        )) {
            msg.errorCode = Error::CSOParser_ValidateHMACFailed;
            msg.data.reset(nullptr);
            return msg;
        }
        return msg;
    }

    // Build aad
    Result<Array<uint8_t>> aad = msg.data->getAad();
    if (aad.errorCode != Error::Nil) {
        msg.errorCode = aad.errorCode;
        msg.data.reset(nullptr);
        return msg;
    }

    // Decypts message
    Array<uint8_t> encryptData;
    encryptData.length = msg.data->getSizeData();
    encryptData.buffer.reset(new (std::nothrow) uint8_t[encryptData.length]);
    if (encryptData.buffer == nullptr) {
        msg.errorCode = Error::NotEnoughMemory;
        msg.data.reset(nullptr);
        return msg;
    }
    Error::Code errorCode = UtilsAES::decrypt(
        this->secretKey.get(), 
        msg.data->getData(), 
        msg.data->getSizeData(), 
        aad.data.buffer.get(), 
        aad.data.length,
        msg.data->getIV(),
        msg.data->getAuthenTag(),
        encryptData.buffer.get()
    );
    if (errorCode != Error::Nil) {
        msg.errorCode = errorCode;
        msg.data.reset(nullptr);
        return msg;
    }
    msg.data->setData(encryptData.buffer.release(), encryptData.length);
    msg.data->setIsEncrypted(false);
    return msg;
}

Result<Array<uint8_t>> Parser::buildActiveMessage(uint16_t ticketID, uint8_t* ticketBytes, uint16_t lenTicket) noexcept {
    String name(ticketID);
    // Build aad
    Result<Array<uint8_t>> aad = Cipher::buildAad(
        0, 
        0, 
        MessageType::Activation, 
        true, 
        true, 
        true, 
        true, 
        name.c_str(), 
        name.length()
    );
    if (aad.errorCode != Error::Nil) {
        return aad;
    }

    // Encrypt ticket
    std::unique_ptr<uint8_t> iv(new (std::nothrow) uint8_t[LENGTH_IV]);
    if (iv == nullptr) {
        return Result<Array<uint8_t>>(Error::NotEnoughMemory, Array<uint8_t>());
    }

    Array<uint8_t> msg;
    msg.length = lenTicket;
    msg.buffer.reset(new (std::nothrow) uint8_t[lenTicket]);
    if (msg.buffer == nullptr) {
        return Result<Array<uint8_t>>(Error::NotEnoughMemory, Array<uint8_t>());
    }

    std::unique_ptr<uint8_t> authenTag(new (std::nothrow) uint8_t[LENGTH_AUTHEN_TAG]);
    if (authenTag == nullptr) {
        return Result<Array<uint8_t>>(Error::NotEnoughMemory, Array<uint8_t>());
    }

    Error::Code errorCode = UtilsAES::encrypt(
        this->secretKey.get(), 
        ticketBytes, 
        lenTicket, 
        aad.data.buffer.get(), 
        aad.data.length,
        iv.get(),
        authenTag.get(),
        msg.buffer.get()
    );
    if (errorCode != Error::Nil) {
        return Result<Array<uint8_t>>(errorCode, Array<uint8_t>());
    }

    // Build cipher bytes
    return Cipher::buildCipherBytes(
        0, 
        0, 
        MessageType::Activation, 
        true, 
        true, 
        true, 
        name.c_str(), 
        name.length(),
        iv.get(),
        msg.buffer.get(),
        msg.length,
        authenTag.get()
    );
}

Result<Array<uint8_t>> Parser::buildMessage(uint64_t msgID, uint64_t msgTag, const char* recvName, uint8_t* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) noexcept {
    return createMessage(
        msgID, 
        msgTag, 
        false, 
        recvName, 
        content, 
        lenContent, 
        encrypted, 
        cache, 
        first, 
        last, 
        request
    );
}

Result<Array<uint8_t>> Parser::buildGroupMessage(uint64_t msgID, uint64_t msgTag, const char* groupName, uint8_t* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) noexcept {
    return createMessage(
        msgID, 
        msgTag, 
        true, 
        groupName, 
        content, 
        lenContent, 
        encrypted, 
        cache, 
        first, 
        last, 
        request
    );
}

MessageType Parser::getMessagetype(bool isGroup, bool isCached) noexcept {
    if (isGroup) {
        if (isCached) {
            return MessageType::GroupCached;
        }
        return MessageType::Group;
    }
    if (isCached) {
        return MessageType::SingleCached;
    }
    return MessageType::Single;
}

Result<Array<uint8_t>> Parser::createMessage(uint64_t msgID, uint64_t msgTag, bool isGroup, const char* name, uint8_t* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) noexcept {
    MessageType msgType = getMessagetype(isGroup, cache);
    if (!encrypted) {
        Result<Array<uint8_t>> rawBytes = Cipher::buildRawBytes(
            msgID, 
            msgTag, 
            msgType, 
            encrypted, 
            first, 
            last, 
            request, 
            name, 
            strlen(name), 
            content, 
            lenContent
        );
        if (rawBytes.errorCode != Error::Nil) {
            return rawBytes;
        }

        std::unique_ptr<uint8_t> sign(new (std::nothrow) uint8_t[LENGTH_SIGN_HMAC]);
        if (sign == nullptr) {
            return Result<Array<uint8_t>>(Error::NotEnoughMemory, Array<uint8_t>());
        }
        
        Error::Code errorCode = UtilsHMAC::calcHMAC(
            this->secretKey.get(), 
            rawBytes.data.buffer.get(), 
            rawBytes.data.length, 
            sign.get()
        );
        if (errorCode != Error::Nil) {
            return Result<Array<uint8_t>>(errorCode, Array<uint8_t>());
        }

        return Cipher::buildNoCipherBytes(
            msgID, 
            msgTag, 
            msgType, 
            first, 
            last, 
            request, 
            name, 
            strlen(name), 
            content, 
            lenContent,
            sign.get()
        );
    }

    // Build aad
    Result<Array<uint8_t>> aad = Cipher::buildAad(
        msgID, 
        msgTag, 
        msgType, 
        true, 
        first, 
        last, 
        request, 
        name, 
        strlen(name)
    );
    if (aad.errorCode != Error::Nil) {
        return aad;
    }

    // Encrypt content
    std::unique_ptr<uint8_t> iv(new (std::nothrow) uint8_t[LENGTH_IV]);
    if (iv == nullptr) {
        return Result<Array<uint8_t>>(Error::NotEnoughMemory, Array<uint8_t>());
    }

    Array<uint8_t> msg;
    msg.length = lenContent;
    msg.buffer.reset(new (std::nothrow) uint8_t[msg.length]);
    if (msg.buffer == nullptr) {
        return Result<Array<uint8_t>>(Error::NotEnoughMemory, Array<uint8_t>());
    }

    std::unique_ptr<uint8_t> authenTag(new (std::nothrow) uint8_t[LENGTH_AUTHEN_TAG]);
    if (authenTag == nullptr) {
        return Result<Array<uint8_t>>(Error::NotEnoughMemory, Array<uint8_t>());
    }

    Error::Code errorCode = UtilsAES::encrypt(
        this->secretKey.get(), 
        content, 
        lenContent, 
        aad.data.buffer.get(), 
        aad.data.length,
        iv.get(),
        authenTag.get(),
        msg.buffer.get()
    );
    if (errorCode != Error::Nil) {
        return Result<Array<uint8_t>>(errorCode, Array<uint8_t>());
    }

    // Build cipher bytes
    return Cipher::buildCipherBytes(
        msgID, 
        msgTag, 
        msgType, 
        first, 
        last, 
        request,
        name, 
        strlen(name),
        iv.get(),
        msg.buffer.get(),
        msg.length,
        authenTag.get()
    );
}