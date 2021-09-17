#include <cstring>
#include "error/error.h"
#include "message/cipher.h"
#include "message/define.h"
#include "utils/utils_define.h"

#define MAX_CONNECTION_NAME_LENGTH 36

Cipher::Cipher() noexcept
    : msgID{ 0 },
      msgTag{ 0 },
      isFirst{ false },
      isLast{ false },
      isRequest{ false },
      isEncrypted{ false },
      iv{ LENGTH_IV },
      authenTag{ LENGTH_AUTHEN_TAG },
      sign{ LENGTH_SIGN },
      data{},
      name{},
      msgType{} {}

Cipher::Cipher(Cipher&& other) noexcept
    : msgID{ std::move(other.msgID) },
      msgTag{ std::move(other.msgTag) },
      isFirst{ std::move(other.isFirst) },
      isLast{ std::move(other.isLast) },
      isRequest{ std::move(other.isRequest) },
      isEncrypted{ std::move(other.isEncrypted) },
      iv{},
      authenTag{},
      sign{},
      data{ std::move(other.data) },
      name{ std::move(other.name) },
      msgType{ std::move(other.msgType) } {
    std::swap(this->iv, other.iv);
    std::swap(this->authenTag, other.authenTag);
    std::swap(this->sign, other.sign);
}

Cipher& Cipher::operator=(Cipher&& other) noexcept {
    this->msgID = std::move(other.msgID);
    this->msgTag = std::move(other.msgTag);
    this->isFirst = std::move(other.isFirst);
    this->isLast = std::move(other.isLast);
    this->isRequest = std::move(other.isRequest);
    this->isEncrypted = std::move(other.isEncrypted);
    this->msgType = std::move(other.msgType);
    this->iv = std::move(other.iv);
    this->authenTag = std::move(other.authenTag);
    this->sign = std::move(other.sign);
    this->data = std::move(other.data);
    this->name = std::move(other.name);
    return *this;
}

void Cipher::setMsgID(uint64_t msgID) noexcept {
    this->msgID = msgID;
}

void Cipher::setMsgTag(uint64_t msgTag) noexcept {
    this->msgTag = msgTag;
}

void Cipher::setMsgType(MessageType msgType) noexcept {
    this->msgType = msgType;
}

void Cipher::setIsFirst(bool isFirst) noexcept {
    this->isFirst = isFirst;
}

void Cipher::setIsLast(bool isLast) noexcept {
    this->isLast = isLast;
}

void Cipher::setIsRequest(bool isRequest) noexcept {
    this->isRequest = isRequest;
}

void Cipher::setIsEncrypted(bool isEncrypted) noexcept {
    this->isEncrypted = isEncrypted;
}

Error Cipher::setIV(const Array<uint8_t>& iv) noexcept {
    if (iv.length() == LENGTH_IV) {
        this->iv = iv;
        return Error{};
    }
    return Error{ GET_FUNC_NAME(), "Length of 'iv' must be 'LENGTH_IV' be defined in 'message/define.h'" };
}

Error Cipher::setAuthenTag(const Array<uint8_t>& authenTag) noexcept {
    if (authenTag.length() == LENGTH_AUTHEN_TAG) {
        this->authenTag = authenTag;
        return Error{};
    }
    return Error{ GET_FUNC_NAME(), "Length of 'authenTag' must be 'LENGTH_AUTHEN_TAG' be defined in 'message/define.h'" };
}

Error Cipher::setSign(const Array<uint8_t>& sign) noexcept {
    if (sign.length() == LENGTH_SIGN) {
        this->sign = sign;
        return Error{};
    }
    return Error{ GET_FUNC_NAME(), "Length of 'sign' must be 'LENGTH_SIGN' be defined in 'message/define.h'" };
}

void Cipher::setName(const std::string& name) {
    this->name = name;
}

void Cipher::setName(std::string&& name) noexcept {
    this->name = name;
}

void Cipher::setData(const Array<uint8_t>& data) {
    this->data = data;
}

void Cipher::setData(Array<uint8_t>&& data) noexcept {
    this->data = data;
}

uint64_t Cipher::getMsgID() const noexcept {
    return this->msgID;
}

uint64_t Cipher::getMsgTag() const noexcept {
    return this->msgTag;
}

MessageType Cipher::getMsgType() const noexcept {
    return this->msgType;
}

bool Cipher::getIsFirst() const noexcept {
    return this->isFirst;
}

bool Cipher::getIsLast() const noexcept {
    return this->isLast;
}

bool Cipher::getIsRequest() const noexcept {
    return this->isRequest;
}

bool Cipher::getIsEncrypted() const noexcept {
    return this->isEncrypted;
}

const Array<uint8_t>& Cipher::getIV() const noexcept {
    return this->iv;
}

const Array<uint8_t>& Cipher::getAuthenTag() const noexcept {
    return this->authenTag;
}

const Array<uint8_t>& Cipher::getSign() const noexcept {
    return this->sign;
}

const std::string& Cipher::getName() const noexcept {
    return this->name;
}

const Array<uint8_t>& Cipher::getData() const noexcept {
    return this->data;
}

std::tuple<Error, Array<uint8_t>> Cipher::intoBytes() {
    if (this->isEncrypted) {
        // Build cipher bytes
        Cipher::buildBytes(
            this->msgID,
            this->msgTag,
            this->msgType,
            true,
            this->isFirst,
            this->isLast,
            this->isRequest,
            this->name,
            this->iv,
            this->authenTag,
            Array<uint8_t>{},
            this->data
        );
    }
    // Build no cipher bytes
    return Cipher::buildBytes(
        this->msgID,
        this->msgTag,
        this->msgType,
        false,
        this->isFirst,
        this->isLast,
        this->isRequest,
        this->name,
        Array<uint8_t>{},
        Array<uint8_t>{},
        this->sign,
        this->data
    );
}

std::tuple<Error, Array<uint8_t>> Cipher::getRawBytes() {
    return Cipher::buildRawBytes(
        this->msgID,
        this->msgTag,
        this->msgType,
        this->isEncrypted,
        this->isFirst,
        this->isLast,
        this->isRequest,
        this->name,
        this->data
    );
}

std::tuple<Error, Array<uint8_t>> Cipher::getAad() {
    return Cipher::buildAad(
        this->msgID,
        this->msgTag,
        this->msgType,
        this->isEncrypted,
        this->isFirst,
        this->isLast,
        this->isRequest,
        this->name
    );
}

// ParseBytes converts bytes to Cipher
// ID of message: 8 bytes
// Encrypted, First, Last, Request/Response, Tag, Type (3 bits): 1 byte
// Length of Name (nName): 1 byte
// Tag: if flag of tag = 1 then 8 bytes, otherwise 0 byte
// AUTHEN_TAG: if encrypted is true then 16 bytes, otherwise 0 byte
// IV: if encrypted is true then 12 bytes, otherwise 0 byte
// Sign: if encrypted is false then 32 bytes (HMAC-SHA256), otherwise 0 byte
// Name: nName bytes
// Data: remaining bytes
std::tuple<Error, std::unique_ptr<Cipher>> Cipher::parseBytes(const  Array<uint8_t>& buffer) {
    uint8_t fixedLen = 10;
    uint8_t posAuthenTag = 10;
    if (buffer.length() < fixedLen) {
        return std::make_tuple(Error{ GET_FUNC_NAME(), "Invalid buffer length" }, nullptr);
    }

    uint8_t flag = buffer[8];
    bool isEncrypted = (flag & 0x80U) != 0;
    uint64_t msgID = ((uint64_t)buffer[7] << 56U) |
                     ((uint64_t)buffer[6] << 48U) |
                     ((uint64_t)buffer[5] << 40U) |
                     ((uint64_t)buffer[4] << 32U) |
                     ((uint64_t)buffer[3] << 24U) |
                     ((uint64_t)buffer[2] << 16U) |
                     ((uint64_t)buffer[1] << 8U) |
                     (uint64_t)buffer[0];

    uint8_t lenName = buffer[9];
    uint64_t msgTag = 0;
    if ((flag & 0x08U) != 0) {
        fixedLen += 8;
        posAuthenTag += 8;
        if (buffer.length() < fixedLen) {
            return std::make_tuple(Error{ GET_FUNC_NAME(), "Invalid buffer length" }, nullptr);
        }
        msgTag = ((uint64_t)buffer[17] << 56U) |
                 ((uint64_t)buffer[16] << 48U) |
                 ((uint64_t)buffer[15] << 40U) |
                 ((uint64_t)buffer[14] << 32U) |
                 ((uint64_t)buffer[13] << 24U) |
                 ((uint64_t)buffer[12] << 16U) |
                 ((uint64_t)buffer[11] << 8U) |
                 (uint64_t)buffer[10];
    }

    if (isEncrypted) {
        fixedLen += LENGTH_AUTHEN_TAG + LENGTH_IV;
    }
    if (lenName == 0 || lenName > MAX_CONNECTION_NAME_LENGTH) {
        return std::make_tuple(Error{ GET_FUNC_NAME(), "Length of name doesn't be over 36" }, nullptr);
    }
    if (buffer.length() < (size_t)fixedLen + (size_t)lenName) {
        return std::make_tuple(Error{ GET_FUNC_NAME(), "Invalid buffer length" }, nullptr);
    }

    std::unique_ptr<Cipher> cipher{ new Cipher() };
    cipher->msgID = msgID;
    cipher->msgTag = msgTag;
    cipher->msgType = (MessageType)(flag & 0x07U);
    cipher->isFirst = (flag & 0x40U) != 0;
    cipher->isLast = (flag & 0x20U) != 0;
    cipher->isRequest = (flag & 0x10U) != 0;
    cipher->isEncrypted = isEncrypted;

    // Parse AUTHEN_TAG, IV
    if (isEncrypted) {
        uint8_t posIV = posAuthenTag + LENGTH_AUTHEN_TAG;
        memcpy(cipher->authenTag.get(), buffer.get() + posAuthenTag, LENGTH_AUTHEN_TAG);
        memcpy(cipher->iv.get(), buffer.get() + posIV, LENGTH_IV);
    }
    else {
        uint8_t posSign = fixedLen;
        fixedLen += LENGTH_SIGN;
        if (buffer.length() < (size_t)fixedLen + (size_t)lenName) {
            return std::make_tuple(Error{ GET_FUNC_NAME(), "Invalid buffer length" }, nullptr);
        }
        memcpy(cipher->sign.get(), buffer.get() + posSign, LENGTH_SIGN);
    }

    // Parse name
    uint8_t posData = fixedLen + lenName;
    cipher->name = std::string{ (char*)buffer.get() + fixedLen, lenName };

    // Parse data
    uint16_t sizeData = buffer.length() - posData;
    if (sizeData > 0) {
        cipher->data.reset(sizeData);
        memcpy(cipher->data.get(), buffer.get() + posData, sizeData);
    }

    return std::make_tuple(Error{}, std::move(cipher));
}

std::tuple<Error, Array<uint8_t>> Cipher::buildRawBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, const std::string& name, const Array<uint8_t>& data) {
    uint8_t lenName = name.length();
    if (lenName == 0 || lenName > MAX_CONNECTION_NAME_LENGTH) {
        return std::make_tuple(Error{ GET_FUNC_NAME(), "Length of name doesn't be over 36" }, Array<uint8_t>{});
    }

    uint8_t bEncrypted = isEncrypted ? 1 : 0;
    uint8_t bFirst = isFirst ? 1 : 0;
    uint8_t bLast = isLast ? 1 : 0;
    uint8_t bRequest = isRequest ? 1 : 0;
    uint8_t bUseTag = 0;
    uint8_t fixedLen = 10;
    if (msgTag > 0) {
        bUseTag = 1;
        fixedLen += 8;
    }

    Array<uint8_t> buffer{ fixedLen + lenName + data.length() };
    buffer[0] = (uint8_t)msgID;
    buffer[1] = (uint8_t)(msgID >> 8U);
    buffer[2] = (uint8_t)(msgID >> 16U);
    buffer[3] = (uint8_t)(msgID >> 24U);
    buffer[4] = (uint8_t)(msgID >> 32U);
    buffer[5] = (uint8_t)(msgID >> 40U);
    buffer[6] = (uint8_t)(msgID >> 48U);
    buffer[7] = (uint8_t)(msgID >> 56U);
    buffer[8] = (uint8_t)(bEncrypted << 7U | bFirst << 6U | bLast << 5U | bRequest << 4U | bUseTag << 3U | uint8_t(msgType));
    buffer[9] = lenName;
    if (msgTag > 0) {
        buffer[10] = (uint8_t)msgTag;
        buffer[11] = (uint8_t)(msgTag >> 8U);
        buffer[12] = (uint8_t)(msgTag >> 16U);
        buffer[13] = (uint8_t)(msgTag >> 24U);
        buffer[14] = (uint8_t)(msgTag >> 32U);
        buffer[15] = (uint8_t)(msgTag >> 40u);
        buffer[16] = (uint8_t)(msgTag >> 48U);
        buffer[17] = (uint8_t)(msgTag >> 56U);
    }
    memcpy(buffer.get() + fixedLen, name.c_str(), lenName);
    if (data.length() > 0) {
        memcpy(buffer.get() + fixedLen + lenName, data.get(), data.length());
    }

    return std::make_tuple(Error{}, std::move(buffer));
}

std::tuple<Error, Array<uint8_t>> Cipher::buildAad(
        uint64_t msgID,
        uint64_t msgTag,
        MessageType msgType,
        bool isEncrypted,
        bool isFirst,
        bool isLast,
        bool isRequest,
        const std::string& name) {
    uint8_t lenName = name.length();

    if (lenName == 0 || lenName > MAX_CONNECTION_NAME_LENGTH) {
        return std::make_tuple(Error{ GET_FUNC_NAME(), "Length of name doesn't be over 36" }, Array<uint8_t>{});
    }

    uint8_t bEncrypted = isEncrypted ? 1 : 0;
    uint8_t bFirst = isFirst ? 1 : 0;
    uint8_t bLast = isLast ? 1 : 0;
    uint8_t bRequest = isRequest ? 1 : 0;
    uint8_t bUseTag = 0;
    uint8_t fixedLen = 10;
    if (msgTag > 0) {
        bUseTag = 1;
        fixedLen += 8;
    }

    Array<uint8_t> buffer{ fixedLen + lenName };
    buffer[0] = (uint8_t)msgID;
    buffer[1] = (uint8_t)(msgID >> 8U);
    buffer[2] = (uint8_t)(msgID >> 16U);
    buffer[3] = (uint8_t)(msgID >> 24U);
    buffer[4] = (uint8_t)(msgID >> 32U);
    buffer[5] = (uint8_t)(msgID >> 40U);
    buffer[6] = (uint8_t)(msgID >> 48U);
    buffer[7] = (uint8_t)(msgID >> 56U);
    buffer[8] = (uint8_t)(bEncrypted << 7U | bFirst << 6U | bLast << 5U | bRequest << 4U | bUseTag << 3U | (uint8_t)msgType);
    buffer[9] = lenName;
    if (msgTag > 0) {
        buffer[10] = (uint8_t)msgTag;
        buffer[11] = (uint8_t)(msgTag >> 8U);
        buffer[12] = (uint8_t)(msgTag >> 16U);
        buffer[13] = (uint8_t)(msgTag >> 24U);
        buffer[14] = (uint8_t)(msgTag >> 32U);
        buffer[15] = (uint8_t)(msgTag >> 40U);
        buffer[16] = (uint8_t)(msgTag >> 48U);
        buffer[17] = (uint8_t)(msgTag >> 56U);
    }
    memcpy(buffer.get() + fixedLen, name.c_str(), lenName);

    return std::make_tuple(Error{}, std::move(buffer));
}

std::tuple<Error, Array<uint8_t>> Cipher::buildCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, const std::string& name, const Array<uint8_t>& iv, const Array<uint8_t>& authenTag, const Array<uint8_t>& data) {
    std::string errContent;

    if (iv.length() != LENGTH_IV) {
        errContent = "Length of 'iv' must be 'LENGTH_IV' be defined in 'message/define.h'";
        goto handleError;
    }

    if (authenTag.length() != LENGTH_AUTHEN_TAG) {
        errContent = "Length of 'authenTag' must be 'LENGTH_AUTHEN_TAG' be defined in 'message/define.h'";
        goto handleError;
    }

    if (name.length() == 0 || name.length() > MAX_CONNECTION_NAME_LENGTH) {
        errContent = "Length of name doesn't be over 36";
        goto handleError;
    }

    return Cipher::buildBytes(
        msgID,
        msgTag,
        msgType,
        true,
        isFirst,
        isLast,
        isRequest,
        name,
        iv,
        authenTag,
        Array<uint8_t>{},
        data
    );

handleError:
    return std::make_tuple(Error{ GET_FUNC_NAME(), std::move(errContent) }, Array<uint8_t>{});
}

std::tuple<Error, Array<uint8_t>> Cipher::buildNoCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, const std::string& name, const Array<uint8_t>& sign, const Array<uint8_t>& data) {
    std::string errContent;

    if (sign.length() != LENGTH_SIGN) {
        errContent = "Length of 'sign' must be 'LENGTH_SIGN' be defined in 'message/define.h'";
        goto handleError;
    }

    if (name.length() == 0 || name.length() > MAX_CONNECTION_NAME_LENGTH) {
        errContent = "Length of name doesn't be over 36";
        goto handleError;
    }

    return Cipher::buildBytes(
        msgID,
        msgTag,
        msgType,
        false,
        isFirst,
        isLast,
        isRequest,
        name,
        Array<uint8_t>{},
        Array<uint8_t>{},
        sign,
        data
    );

handleError:
    return std::make_tuple(Error{ GET_FUNC_NAME(), std::move(errContent) }, Array<uint8_t>{});
}

std::tuple<Error, Array<uint8_t>> Cipher::buildBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, const std::string& name, const Array<uint8_t>& iv, const Array<uint8_t>& authenTag, const Array<uint8_t>& sign, const Array<uint8_t>& data) {
    uint8_t lenName = name.length();
    uint8_t bEncrypted = isEncrypted ? 1 : 0;
    uint8_t bFirst = isFirst ? 1 : 0;
    uint8_t bLast = isLast ? 1 : 0;
    uint8_t bRequest = isRequest ? 1 : 0;
    uint8_t bUseTag = 0;
    uint8_t fixedLen = 10;
    if (msgTag > 0) {
        bUseTag = 1;
        fixedLen += 8;
    }

    uint16_t lenBuffer = fixedLen + lenName + data.length();
    if (isEncrypted) {
        lenBuffer += (LENGTH_IV + LENGTH_AUTHEN_TAG);
    }
    else {
        lenBuffer += LENGTH_SIGN;
    }

    uint8_t* buffer = new uint8_t[lenBuffer];
    buffer[0] = (uint8_t)msgID;
    buffer[1] = (uint8_t)(msgID >> 8U);
    buffer[2] = (uint8_t)(msgID >> 16U);
    buffer[3] = (uint8_t)(msgID >> 24U);
    buffer[4] = (uint8_t)(msgID >> 32U);
    buffer[5] = (uint8_t)(msgID >> 40U);
    buffer[6] = (uint8_t)(msgID >> 48U);
    buffer[7] = (uint8_t)(msgID >> 56U);
    buffer[8] = (uint8_t)(bEncrypted << 7U | bFirst << 6U | bLast << 5U | bRequest << 4U | bUseTag << 3U | (uint8_t)msgType);
    buffer[9] = lenName;
    if (msgTag > 0) {
        buffer[10] = (uint8_t)msgTag;
        buffer[11] = (uint8_t)(msgTag >> 8U);
        buffer[12] = (uint8_t)(msgTag >> 16U);
        buffer[13] = (uint8_t)(msgTag >> 24U);
        buffer[14] = (uint8_t)(msgTag >> 32U);
        buffer[15] = (uint8_t)(msgTag >> 40U);
        buffer[16] = (uint8_t)(msgTag >> 48U);
        buffer[17] = (uint8_t)(msgTag >> 56U);
    }
    uint8_t posData = fixedLen;
    if (isEncrypted) {
        memcpy(buffer + fixedLen, authenTag.get(), LENGTH_AUTHEN_TAG);
        posData += LENGTH_AUTHEN_TAG;
        memcpy(buffer + posData, iv.get(), LENGTH_IV);
        posData += LENGTH_IV;
    }
    else {
        memcpy(buffer + fixedLen, sign.get(), LENGTH_SIGN);
        posData += LENGTH_SIGN;
    }
    memcpy(buffer + posData, name.c_str(), lenName);
    posData += lenName;
    if (data.length() > 0) {
        memcpy(buffer + posData, data.get(), data.length());
    }
    return std::make_tuple(Error{}, Array<uint8_t>{ buffer, lenBuffer });
}