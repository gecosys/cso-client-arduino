#include <WString.h>
#include "cso_parser/parser.h"
#include "utils/utils_aes.h"
#include "utils/utils_hmac.h"
#include "utils/utils_general.hpp"

std::unique_ptr<IParser> Parser::build() {
    return std::unique_ptr<IParser>{ new Parser{} };
}

void Parser::setSecretKey(Array<uint8_t>&& secretKey) noexcept {
    this->secretKey = std::forward<Array<uint8_t>>(secretKey);
}

void Parser::setSecretKey(const Array<uint8_t>& secretKey) {
    this->secretKey = secretKey;
}

std::tuple<Error, std::unique_ptr<Cipher>> Parser::parseReceivedMessage(const Array<uint8_t>& content) {
    // Parse message
    Error err;
    std::unique_ptr<Cipher> cipher;

    std::tie(err, cipher) = Cipher::parseBytes(content);
    if (!err.nil()) {
        return std::make_tuple(std::move(err), nullptr);
    }

    // Solve if message is not encrypted
    if (!cipher->getIsEncrypted()) {
        bool valid;
        Array<uint8_t> rawBytes;

        std::tie(err, rawBytes) = cipher->getRawBytes();
        if (!err.nil()) {
            return std::make_tuple(std::move(err), nullptr);
        }

        std::tie(err, valid) = UtilsHMAC::validateHMAC(this->secretKey, rawBytes, cipher->getSign());
        if (!err.nil()) {
            return std::make_tuple(std::move(err), nullptr);
        }
        if (!valid) {
            return std::make_tuple(Error{ GET_FUNC_NAME(), "HMAC authentication failed" }, nullptr);
        }
        return std::make_tuple(std::move(err), std::move(cipher));
    }

    // Decypts message
    Array<uint8_t> decryptData;
    {
        // Build aad
        Array<uint8_t> aad;

        std::tie(err, aad) = cipher->getAad();
        if (!err.nil()) {
            return std::make_tuple(std::move(err), nullptr);
        }

        std::tie(err, decryptData) = UtilsAES::decrypt(this->secretKey, cipher->getData(), aad, cipher->getIV(), cipher->getAuthenTag());
        if (!err.nil()) {
            return std::make_tuple(std::move(err), nullptr);
        }
    }
    cipher->setData(std::move(decryptData));
    cipher->setIsEncrypted(false);
    return std::make_tuple(std::move(err), std::move(cipher));
}

std::tuple<Error, Array<uint8_t>> Parser::buildActiveMessage(uint16_t ticketID, const Array<uint8_t>& ticketBytes) {
    std::string name(String(ticketID).c_str());

    // Build aad
    Error err;
    Array<uint8_t> aad;

    std::tie(err, aad) = Cipher::buildAad(0, 0, MessageType::Activation, true, true, true, true, name);
    if (!err.nil()) {
        return std::make_tuple(std::move(err), Array<uint8_t>{});
    }

    // Encrypt ticket
    Array<uint8_t> iv;
    Array<uint8_t> authenTag;
    Array<uint8_t> ticket;

    std::tie(err, iv, authenTag, ticket) = UtilsAES::encrypt(this->secretKey, ticketBytes, aad);
    if (!err.nil()) {
        return std::make_tuple(std::move(err), Array<uint8_t>{});
    }

    // Build cipher bytes
    return Cipher::buildCipherBytes(0, 0, MessageType::Activation, true, true, true, name, iv, authenTag, ticket);
}

std::tuple<Error, Array<uint8_t>> Parser::buildMessage(uint64_t msgID, uint64_t msgTag, bool encrypted, bool cache, bool first, bool last, bool request, const std::string& recvName, const Array<uint8_t>& content) {
    return createMessage(msgID, msgTag, false, encrypted, cache, first, last, request, recvName, content);
}

std::tuple<Error, Array<uint8_t>> Parser::buildGroupMessage(uint64_t msgID, uint64_t msgTag, bool encrypted, bool cache, bool first, bool last, bool request, const std::string& groupName, const Array<uint8_t>& content) {
    return createMessage(msgID, msgTag, true, encrypted, cache, first, last, request, groupName, content);
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

std::tuple<Error, Array<uint8_t>> Parser::createMessage(uint64_t msgID, uint64_t msgTag, bool isGroup, bool encrypted, bool cache, bool first, bool last, bool request, const std::string& name, const Array<uint8_t>& content) {
    //return { Error::Nil, Array < uint8_t>{} };
    MessageType msgType = getMessagetype(isGroup, cache);
    if (!encrypted) {
        // Build raw bytes
        Error err;
        Array<uint8_t> rawBytes;

        std::tie(err, rawBytes) = Cipher::buildRawBytes(msgID, msgTag, msgType, encrypted, first, last, request, name, content);
        if (!err.nil()) {
            return std::make_tuple(std::move(err), Array<uint8_t>{});
        }

        // Build signature
        Array<uint8_t> signature;
        std::tie(err, signature) = UtilsHMAC::calcHMAC(this->secretKey, rawBytes);
        if (!err.nil()) {
            return std::make_tuple(std::move(err), Array<uint8_t>{});
        }

        // Build no cipher bytes
        return Cipher::buildNoCipherBytes(msgID, msgTag, msgType, first, last, request, name, signature, content);
    }

    // Build aad
    Error err;
    Array<uint8_t> aad;

    std::tie(err, aad) = Cipher::buildAad(msgID, msgTag, msgType, true, first, last, request, name);
    if (!err.nil()) {
        return std::make_tuple(std::move(err), Array<uint8_t>{});
    }

    // Encrypt content
    Array<uint8_t> iv;
    Array<uint8_t> authenTag;
    Array<uint8_t> encyptData;

    std::tie(err, iv, authenTag, encyptData) = UtilsAES::encrypt(this->secretKey, content, aad);
    if (!err.nil()) {
        return std::make_tuple(std::move(err), Array<uint8_t>{});
    }

    // Build cipher bytes
    return Cipher::buildCipherBytes(msgID, msgTag, msgType, first, last, request, name, iv, authenTag, encyptData);
}