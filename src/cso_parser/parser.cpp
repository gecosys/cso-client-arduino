#include "cso_parser/parser.h"
#include "utils/utils_aes.h"
#include "utils/utils_hmac.h"

std::shared_ptr<IParser> Parser::build() {
    IParser* obj = Safe::new_obj<Parser>();
    if (obj == nullptr) {
        throw std::runtime_error("[cso_parser/Parser::build(...)]Not enough memory to create object");
    }
    return std::shared_ptr<IParser>(obj);
}

Parser::~Parser() {}

void Parser::setSecretKey(std::shared_ptr<byte> secretKey) noexcept {
    this->secretKey.swap(secretKey);
}

std::pair<Error::Code, std::shared_ptr<Cipher>> Parser::parseReceivedMessage(byte* content, uint16_t lenContent) {
    // Parse message
    Result<std::shared_ptr<Cipher>> msg = Cipher::parseBytes(content, lenContent);
    if (msg.errorCode != SUCCESS) {
        return std::pair<Error::Code, std::shared_ptr<Cipher>>(Error::Parse, std::shared_ptr<Cipher>(nullptr));
    }

    // Solve if message is not encrypted
    if (!msg.data->getIsEncrypted()) {
        Result<Array<byte>> rawBytes = msg.data->getRawBytes();
        if (rawBytes.errorCode != SUCCESS) {
            return std::pair<Error::Code, std::shared_ptr<Cipher>>(Error::Build, std::shared_ptr<Cipher>(nullptr));
        }

        if (!UtilsHMAC::validateHMAC(
            this->secretKey.get(), 
            rawBytes.data.ptr.get(), 
            rawBytes.data.length, 
            msg.data->getSign()
        )) {
            return std::pair<Error::Code, std::shared_ptr<Cipher>>(Error::Verify, std::shared_ptr<Cipher>(nullptr));
        }
        return std::pair<Error::Code, std::shared_ptr<Cipher>>(Error::Nil, msg.data);
    }

    // Build aad
    Result<Array<byte>> aad = msg.data->getAad();
    if (aad.errorCode != SUCCESS) {
        return std::pair<Error::Code, std::shared_ptr<Cipher>>(Error::Build, std::shared_ptr<Cipher>(nullptr));
    }

    // Decypts message
    std::unique_ptr<byte> encryptData(Safe::new_arr<byte>(LENGTH_OUTPUT));
    if (encryptData.get() == nullptr) {
        return std::pair<Error::Code, std::shared_ptr<Cipher>>(Error::NotEnoughMem, std::shared_ptr<Cipher>(nullptr));
    }
    if (UtilsAES::decrypt(
        this->secretKey.get(), 
        msg.data->getData(), 
        msg.data->getSizeData(), 
        aad.data.ptr.get(), 
        aad.data.length,
        msg.data->getIV(),
        msg.data->getAuthenTag(),
        encryptData.get()
    ) != SUCCESS) {
        return std::pair<Error::Code, std::shared_ptr<Cipher>>(Error::Decrypt, std::shared_ptr<Cipher>(nullptr));
    }
    msg.data->setData(encryptData.get(), LENGTH_OUTPUT);
    msg.data->setIsEncrypted(false);
    return std::pair<Error::Code, std::shared_ptr<Cipher>>(Error::Nil, msg.data);
}

std::pair<Error::Code, Array<byte>> Parser::buildActiveMessage(uint16_t ticketID, byte* ticketBytes, uint16_t lenTicket) {
    String name(ticketID);
    // Build aad
    Result<Array<byte>> aad = Cipher::buildAad(
        0, 0, MessageType::Activation, 
        true, true, true, true, 
        name.c_str(), name.length()
    );
    if (aad.errorCode != SUCCESS) {
        return std::pair<Error::Code, Array<byte>>(Error::Build, Array<byte>());
    }

    // Encrypt ticket
    std::unique_ptr<byte> iv(Safe::new_arr<byte>(LENGTH_IV));
    if (iv.get() == nullptr) {
        return std::pair<Error::Code, Array<byte>>(Error::NotEnoughMem, Array<byte>());
    }

    std::unique_ptr<byte> data(Safe::new_arr<byte>(LENGTH_OUTPUT));
    if (data.get() == nullptr) {
        return std::pair<Error::Code, Array<byte>>(Error::NotEnoughMem, Array<byte>());
    }

    std::unique_ptr<byte> authenTag(Safe::new_arr<byte>(LENGTH_AUTHEN_TAG));
    if (authenTag.get() == nullptr) {
        return std::pair<Error::Code, Array<byte>>(Error::NotEnoughMem, Array<byte>());
    }

    if (UtilsAES::encrypt(
        this->secretKey.get(), 
        ticketBytes, 
        lenTicket, 
        aad.data.ptr.get(), 
        aad.data.length,
        iv.get(),
        authenTag.get(),
        data.get()
    ) != SUCCESS) {
        return std::pair<Error::Code, Array<byte>>(Error::Encrypt, Array<byte>());
    }

    // Build cipher bytes
    Result<Array<byte>> cipher = Cipher::buildCipherBytes(
        0, 0, MessageType::Activation, 
        true, true, true, 
        name.c_str(), name.length(),
        iv.get(),
        data.get(),
        LENGTH_OUTPUT,
        authenTag.get()
    );
    if (cipher.errorCode != SUCCESS) {
        return std::pair<Error::Code, Array<byte>>(Error::Build, Array<byte>());
    }
    return std::pair<Error::Code, Array<byte>>(Error::Nil, cipher.data);
}

std::pair<Error::Code, Array<byte>> Parser::buildMessage(uint64_t msgID, uint64_t msgTag, const char* recvName, byte* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) {
    return createMessage(
        msgID, msgTag, 
        false, 
        recvName, 
        content, lenContent, 
        encrypted, cache, first, last, request
    );
}

std::pair<Error::Code, Array<byte>> Parser::buildGroupMessage(uint64_t msgID, uint64_t msgTag, const char* groupName, byte* content, uint16_t lenContent, bool encrypted, bool cache, bool first, bool last, bool request) {
    return createMessage(
        msgID, msgTag, 
        true, 
        groupName, 
        content, lenContent, 
        encrypted, cache, first, last, request
    );
}

MessageType Parser::getMessagetype(bool isGroup, bool isCached) {
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

std::pair<Error::Code, Array<byte>> Parser::createMessage(uint64_t msgID, uint64_t msgTag,bool isGroup,const char* name,byte* content,uint16_t lenContent,bool encrypted,bool cache,bool first,bool last,bool request) {
    MessageType msgType = getMessagetype(isGroup, cache);
    if (!encrypted) {
        Result<Array<byte>> rawBytes = Cipher::buildRawBytes(
            msgID, msgTag, msgType, 
            encrypted, first, last, request, 
            name, strlen(name), 
            content, lenContent
        );
        if (rawBytes.errorCode != SUCCESS) {
            std::pair<Error::Code, Array<byte>>(Error::Build, Array<byte>());
        }

        std::unique_ptr<byte> sign(Safe::new_arr<byte>(LENGTH_SIGN_HMAC));
        if (sign.get() == nullptr) {
            std::pair<Error::Code, Array<byte>>(Error::NotEnoughMem, Array<byte>());
        }
        
        if (UtilsHMAC::calcHMAC(
            this->secretKey.get(), 
            rawBytes.data.ptr.get(), 
            rawBytes.data.length, 
            sign.get()
        ) != SUCCESS) {
            std::pair<Error::Code, Array<byte>>(Error::Build, Array<byte>());
        }

        Result<Array<byte>> noCipher = Cipher::buildNoCipherBytes(
            msgID, msgTag, msgType, 
            first, last, request, 
            name, strlen(name), 
            content, lenContent,
            sign.get()
        );
        if (noCipher.errorCode != SUCCESS) {
            return std::pair<Error::Code, Array<byte>>(Error::Build, Array<byte>());
        }
        return std::pair<Error::Code, Array<byte>>(Error::Nil, noCipher.data);
    }

    // Build aad
    Result<Array<byte>> aad = Cipher::buildAad(
        msgID, msgTag, msgType, 
        true, first, last, request, 
        name, strlen(name)
    );
    if (aad.errorCode != SUCCESS) {
        std::pair<Error::Code, Array<byte>>(Error::Build, Array<byte>());
    }

    // Encrypt content
    std::unique_ptr<byte> iv(Safe::new_arr<byte>(LENGTH_IV));
    if (iv.get() == nullptr) {
        return std::pair<Error::Code, Array<byte>>(Error::NotEnoughMem, Array<byte>());
    }

    std::unique_ptr<byte> data(Safe::new_arr<byte>(LENGTH_OUTPUT));
    if (data.get() == nullptr) {
        return std::pair<Error::Code, Array<byte>>(Error::NotEnoughMem, Array<byte>());
    }

    std::unique_ptr<byte> authenTag(Safe::new_arr<byte>(LENGTH_AUTHEN_TAG));
    if (authenTag.get() == nullptr) {
        return std::pair<Error::Code, Array<byte>>(Error::NotEnoughMem, Array<byte>());
    }

    if (UtilsAES::encrypt(
        this->secretKey.get(), 
        content, 
        lenContent, 
        aad.data.ptr.get(), 
        aad.data.length,
        iv.get(),
        authenTag.get(),
        data.get()
    ) != SUCCESS) {
        return std::pair<Error::Code, Array<byte>>(Error::Encrypt, Array<byte>());
    }

    // Build cipher bytes
    Result<Array<byte>> cipher = Cipher::buildCipherBytes(
        msgID, msgTag, msgType, 
        first, last, request,
        name, strlen(name),
        iv.get(),
        data.get(),
        LENGTH_OUTPUT,
        authenTag.get()
    );
    if (cipher.errorCode != SUCCESS) {
        return std::pair<Error::Code, Array<byte>>(Error::Build, Array<byte>());
    }
    return std::pair<Error::Code, Array<byte>>(Error::Nil, cipher.data);
}