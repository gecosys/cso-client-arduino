#include "cso_parser/parser.h"
#include "utils/utils_aes.h"
#include "utils/utils_hmac.h"

std::unique_ptr<IParser> Parser::build() {
    IParser* obj = new (std::nothrow) Parser();
    if (obj == nullptr) {
        throw std::runtime_error("[cso_parser/Parser::build(...)]Not enough memory to create object");
    }
    return std::unique_ptr<IParser>(obj);
}

Parser::~Parser() noexcept {}

void Parser::setSecretKey(std::shared_ptr<byte> secretKey) noexcept {
    this->secretKey.swap(secretKey);
}

std::pair<Error::Code, std::unique_ptr<Cipher>> Parser::parseReceivedMessage(byte* content, uint16_t lenContent) noexcept {
    // Parse message
    Result<std::unique_ptr<Cipher>> msg = Cipher::parseBytes(content, lenContent);
    if (msg.errorCode != Error::Nil) {
        return std::make_pair(msg.errorCode, std::unique_ptr<Cipher>(nullptr));
    }

    // Solve if message is not encrypted
    if (!msg.data->getIsEncrypted()) {
        Result<Array<byte>> rawBytes = msg.data->getRawBytes();
        if (rawBytes.errorCode != Error::Nil) {
            return std::make_pair(rawBytes.errorCode, std::unique_ptr<Cipher>(nullptr));
        }

        if (!UtilsHMAC::validateHMAC(
            this->secretKey.get(), 
            rawBytes.data.buffer.get(), 
            rawBytes.data.length, 
            msg.data->getSign()
        )) {
            return std::make_pair(Error::CSOParser_ValidateHMACFailed, std::unique_ptr<Cipher>(nullptr));
        }
        return std::make_pair(Error::Nil, std::move(msg.data));
    }

    // Build aad
    Result<Array<byte>> aad = msg.data->getAad();
    if (aad.errorCode != Error::Nil) {
        return std::make_pair(aad.errorCode, std::unique_ptr<Cipher>(nullptr));
    }

    // Decypts message
    Array<byte> encryptData;
    encryptData.length = msg.data->getSizeData();
    encryptData.buffer.reset(new (std::nothrow) byte[encryptData.length]);
    if (encryptData.buffer == nullptr) {
        return std::make_pair(Error::NotEnoughMemory, std::unique_ptr<Cipher>(nullptr));
    }
    Error::Code error = UtilsAES::decrypt(
        this->secretKey.get(), 
        msg.data->getData(), 
        msg.data->getSizeData(), 
        aad.data.buffer.get(), 
        aad.data.length,
        msg.data->getIV(),
        msg.data->getAuthenTag(),
        encryptData.buffer.get()
    );
    if (error != Error::Nil) {
        return std::make_pair(error, std::unique_ptr<Cipher>(nullptr));
    }
    msg.data->setData(encryptData.buffer.release(), encryptData.length);
    msg.data->setIsEncrypted(false);
    return std::make_pair(Error::Nil, std::move(msg.data));
}

std::pair<Error::Code, Array<byte>> Parser::buildActiveMessage(uint16_t ticketID, byte* ticketBytes, uint16_t lenTicket) noexcept {
    String name(ticketID);
    // Build aad
    Result<Array<byte>> aad = Cipher::buildAad(
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
        return std::make_pair(aad.errorCode, Array<byte>());
    }

    // Encrypt ticket
    std::unique_ptr<byte> iv(new (std::nothrow) byte[LENGTH_IV]);
    if (iv == nullptr) {
        return std::make_pair(Error::NotEnoughMemory, Array<byte>());
    }

    Array<byte> msg;
    msg.length = lenTicket;
    msg.buffer.reset(new (std::nothrow) byte[lenTicket]);
    if (msg.buffer == nullptr) {
        return std::make_pair(Error::NotEnoughMemory, Array<byte>());
    }

    std::unique_ptr<byte> authenTag(new (std::nothrow) byte[LENGTH_AUTHEN_TAG]);
    if (authenTag == nullptr) {
        return std::make_pair(Error::NotEnoughMemory, Array<byte>());
    }

    Error::Code error = UtilsAES::encrypt(
        this->secretKey.get(), 
        ticketBytes, 
        lenTicket, 
        aad.data.buffer.get(), 
        aad.data.length,
        iv.get(),
        authenTag.get(),
        msg.buffer.get()
    );
    if (error != Error::Nil) {
        return std::make_pair(error, Array<byte>());
    }

    // Build cipher bytes
    Result<Array<byte>> cipher = Cipher::buildCipherBytes(
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
    if (cipher.errorCode != Error::Nil) {
        return std::make_pair(cipher.errorCode, Array<byte>());
    }
    return std::make_pair(Error::Nil, cipher.data);
}

std::pair<Error::Code, Array<byte>> Parser::buildMessage(uint64_t msgID, uint64_t msgTag, const char* recvName, byte* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) noexcept {
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

std::pair<Error::Code, Array<byte>> Parser::buildGroupMessage(uint64_t msgID, uint64_t msgTag, const char* groupName, byte* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) noexcept {
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

std::pair<Error::Code, Array<byte>> Parser::createMessage(uint64_t msgID, uint64_t msgTag,bool isGroup,const char* name,byte* content,uint16_t lenContent,bool encrypted,bool cache,bool first,bool last,bool request) noexcept {
    MessageType msgType = getMessagetype(isGroup, cache);
    if (!encrypted) {
        Result<Array<byte>> rawBytes = Cipher::buildRawBytes(
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
            return std::make_pair(rawBytes.errorCode, Array<byte>());
        }

        std::unique_ptr<byte> sign(new (std::nothrow) byte[LENGTH_SIGN_HMAC]);
        if (sign == nullptr) {
            return std::make_pair(Error::NotEnoughMemory, Array<byte>());
        }
        
        Error::Code error = UtilsHMAC::calcHMAC(
            this->secretKey.get(), 
            rawBytes.data.buffer.get(), 
            rawBytes.data.length, 
            sign.get()
        );
        if (error != Error::Nil) {
            std::make_pair(error, Array<byte>());
        }

        Result<Array<byte>> noCipher = Cipher::buildNoCipherBytes(
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
        if (noCipher.errorCode != Error::Nil) {
            return std::make_pair(noCipher.errorCode, Array<byte>());
        }
        return std::make_pair(Error::Nil, noCipher.data);
    }

    // Build aad
    Result<Array<byte>> aad = Cipher::buildAad(
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
        return std::make_pair(aad.errorCode, Array<byte>());
    }

    // Encrypt content
    std::unique_ptr<byte> iv(new (std::nothrow) byte[LENGTH_IV]);
    if (iv == nullptr) {
        return std::make_pair(Error::NotEnoughMemory, Array<byte>());
    }

    Array<byte> msg;
    msg.length = lenContent;
    msg.buffer.reset(new (std::nothrow) byte[msg.length]);
    if (msg.buffer == nullptr) {
        return std::make_pair(Error::NotEnoughMemory, Array<byte>());
    }

    std::unique_ptr<byte> authenTag(new (std::nothrow) byte[LENGTH_AUTHEN_TAG]);
    if (authenTag == nullptr) {
        return std::make_pair(Error::NotEnoughMemory, Array<byte>());
    }

    Error::Code error = UtilsAES::encrypt(
        this->secretKey.get(), 
        content, 
        lenContent, 
        aad.data.buffer.get(), 
        aad.data.length,
        iv.get(),
        authenTag.get(),
        msg.buffer.get()
    );
    if (error != Error::Nil) {
        return std::make_pair(error, Array<byte>());
    }

    // Build cipher bytes
    Result<Array<byte>> cipher = Cipher::buildCipherBytes(
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
    if (cipher.errorCode != Error::Nil) {
        return std::make_pair(cipher.errorCode, Array<byte>());
    }
    return std::make_pair(Error::Nil, cipher.data);
}