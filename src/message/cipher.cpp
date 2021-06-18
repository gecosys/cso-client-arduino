#include <cstring>
#include "message/cipher.h"
#include "message/define.h"

#define MAX_CONNECTION_NAME_LENGTH 36

Cipher::Cipher() noexcept
 : msgID(-1),
   msgTag(-1),
   isFirst(false),
   isLast(false),
   isRequest(false),
   isEncrypted(false),
   iv(),
   sign(),
   authenTag(),
   sizeData(0),
   data(nullptr),
   lenName(0),
   name(nullptr),
   msgType() {}

Cipher::Cipher(Cipher&& other) noexcept
 : msgID(other.msgID),
   msgTag(other.msgTag),
   isFirst(other.isFirst),
   isLast(other.isLast),
   isRequest(other.isRequest),
   isEncrypted(other.isEncrypted),
   iv(),
   sign(),
   authenTag(),
   sizeData(other.sizeData),
   data(nullptr),
   lenName(other.lenName),
   name(nullptr),
   msgType(other.msgType) {
    memcpy(this->iv, other.iv, LENGTH_IV);
    memcpy(this->sign, other.sign, LENGTH_SIGN_HMAC);
    memcpy(this->authenTag, other.authenTag, LENGTH_AUTHEN_TAG);
    std::swap(this->data, other.data);
    std::swap(this->name, other.name);
}

Cipher::~Cipher() noexcept {
    if (this->name != nullptr) {
        delete []this->name;
    }
    if (this->data != nullptr) {
        delete []this->data;
    }
}

Cipher& Cipher::operator=(Cipher&& other) noexcept {
    this->msgID = other.msgID;
    this->msgTag = other.msgTag;
    this->isFirst = other.isFirst;
    this->isLast = other.isLast;
    this->isRequest = other.isRequest;
    this->isEncrypted = other.isEncrypted;
    this->sizeData = other.sizeData;
    this->lenName = other.lenName;
    this->msgType = other.msgType;
    memcpy(this->iv, other.iv, LENGTH_IV);
    memcpy(this->sign, other.sign, LENGTH_SIGN_HMAC);
    memcpy(this->authenTag, other.authenTag, LENGTH_AUTHEN_TAG);
    std::swap(this->data, other.data);
    std::swap(this->name, other.name);
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

void Cipher::setIV(uint8_t iv[LENGTH_IV]) noexcept {
    memcpy(this->iv, iv, LENGTH_IV);
}

void Cipher::setSign(uint8_t sign[LENGTH_SIGN_HMAC]) noexcept {
    memcpy(this->sign, sign, LENGTH_SIGN_HMAC);
}

void Cipher::setAuthenTag(uint8_t authenTag[LENGTH_AUTHEN_TAG]) noexcept {
    memcpy(this->authenTag, authenTag, LENGTH_AUTHEN_TAG);
}

void Cipher::setName(char* name, uint8_t lenName) noexcept {
    this->lenName = lenName;
    if (this->name != nullptr) {
        delete []this->name;
    }
    this->name = name;
}

void Cipher::setData(uint8_t* data, uint16_t sizeData) noexcept {
    this->sizeData = sizeData;
    if (this->data != nullptr) {
        delete []this->data;
    }
    this->data = data;
}


uint64_t Cipher::getMsgID() noexcept {
    return this->msgID;
}

uint64_t Cipher::getMsgTag() noexcept {
    return this->msgTag;
}

MessageType Cipher::getMsgType() noexcept {
    return this->msgType;
}

bool Cipher::getIsFirst() noexcept {
    return this->isFirst;
}

bool Cipher::getIsLast() noexcept {
    return this->isLast;
}

bool Cipher::getIsRequest() noexcept {
    return this->isRequest;
}

bool Cipher::getIsEncrypted() noexcept {
    return this->isEncrypted;
}

uint8_t* Cipher::getIV() noexcept {
    return this->iv;
}

uint8_t* Cipher::getSign() noexcept {
    return this->sign;
}

uint8_t* Cipher::getAuthenTag() noexcept {
    return this->authenTag;
}

char* Cipher::getName() noexcept {
    return this->name;
}

uint8_t Cipher::getLengthName() noexcept {
    return this->lenName;
}

uint8_t* Cipher::getData() noexcept {
    return this->data;
}

uint16_t Cipher::getSizeData() noexcept {
    return this->sizeData;
}

Result<Array<uint8_t>> Cipher::intoBytes() noexcept {
    if (this->isEncrypted) {
        return Cipher::buildCipherBytes(
            this->msgID,
            this->msgTag,
            this->msgType,
            this->isFirst,
            this->isLast,
            this->isRequest,
            this->name,
            this->lenName,
            this->iv,
            this->data,
            this->sizeData,
            this->authenTag
        );
    }
    return Cipher::buildNoCipherBytes(
            this->msgID,
            this->msgTag,
            this->msgType,
            this->isFirst,
            this->isLast,
            this->isRequest,
            this->name,
            this->lenName,
            this->data,
            this->sizeData,
            this->sign
        );
}

Result<Array<uint8_t>> Cipher::getRawBytes() noexcept {
    return Cipher::buildRawBytes(
        this->msgID,
        this->msgTag,
        this->msgType,
        this->isEncrypted,
        this->isFirst,
        this->isLast,
        this->isRequest,
        this->name,
        this->lenName,
        this->data,
        this->sizeData
    );
}

Result<Array<uint8_t>> Cipher::getAad() noexcept {
    return Cipher::buildAad(
        this->msgID,
        this->msgTag,
        this->msgType,
        this->isEncrypted,
        this->isFirst,
        this->isLast,
        this->isRequest,
        this->name,
        this->lenName
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
Result<std::unique_ptr<Cipher>> Cipher::parseBytes(uint8_t* buffer, uint16_t sizeBuffer) noexcept {
    Result<std::unique_ptr<Cipher>> result;
    uint8_t fixedLen = 10;
    uint8_t posAuthenTag = 10;
    if (sizeBuffer < fixedLen) {
        result.errorCode = Error::Message_InvalidBytes;
        return result;
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
        if (sizeBuffer < fixedLen) {
            result.errorCode = Error::Message_InvalidBytes;
            return result;
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
		result.errorCode = Error::Message_InvalidConnectionName;
        return result;
	}
    if (sizeBuffer < fixedLen + lenName) {
		result.errorCode = Error::Message_InvalidBytes;
        return result;
	}

    // Parse AUTHEN_TAG, IV
    uint8_t iv[LENGTH_IV];
    uint8_t sign[LENGTH_SIGN_HMAC];
    uint8_t authenTag[LENGTH_AUTHEN_TAG];
    if (isEncrypted) {
        uint8_t posIV = posAuthenTag + LENGTH_AUTHEN_TAG;
        memcpy(authenTag, buffer + posAuthenTag, LENGTH_AUTHEN_TAG);
		memcpy(iv, buffer+posIV, LENGTH_IV);
    } else {
        uint8_t posSign = fixedLen;
        fixedLen += LENGTH_SIGN_HMAC;
        if (sizeBuffer < fixedLen + lenName) {
            result.errorCode = Error::Message_InvalidBytes;
            return result;
        }
        memcpy(sign, buffer + posSign, LENGTH_SIGN_HMAC);
    }

    // Parse name
    uint8_t posData = fixedLen + lenName;
    char* name = new (std::nothrow) char[lenName + 1];
    if (name == nullptr) {
        result.errorCode = Error::NotEnoughMemory;
        return result;
    }
    name[lenName] = '\0';
    memcpy(name, buffer + fixedLen, lenName);

    // Parse data
    uint8_t* data = nullptr;
    uint16_t sizeData = sizeBuffer - posData;
    if (sizeData > 0) {
		data = new (std::nothrow) uint8_t[sizeData];
        if (data == nullptr) {
            result.errorCode = Error::NotEnoughMemory;
            return result;
        }
		memcpy(data, buffer + posData, sizeData);
	}

    Cipher* cipher = new Cipher();
    if (cipher == nullptr) {
        result.errorCode = Error::NotEnoughMemory;
        return result;
    }
    cipher->msgID = msgID;
    cipher->msgTag = msgTag;
    cipher->msgType = (MessageType)(flag & 0x07U);
    cipher->isFirst = (flag & 0x40U) != 0;
    cipher->isLast = (flag & 0x20U) != 0;
    cipher->isRequest = (flag & 0x10U) != 0;
    cipher->isEncrypted = isEncrypted;
    memcpy(cipher->iv, iv, LENGTH_IV);
    memcpy(cipher->sign, sign, LENGTH_SIGN_HMAC);
    memcpy(cipher->authenTag, authenTag, LENGTH_AUTHEN_TAG);
    cipher->sizeData = sizeData;
    cipher->data = data;
    cipher->lenName = lenName;
    cipher->name = name;

    result.data.reset(cipher);
    result.errorCode = Error::Nil;
    return result;
}

Result<Array<uint8_t>> Cipher::buildRawBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, const char* name, uint8_t lenName, uint8_t* data, uint16_t sizeData) noexcept {
    Result<Array<uint8_t>> result;
    if (lenName == 0 || lenName > MAX_CONNECTION_NAME_LENGTH) {
		result.errorCode = Error::Message_InvalidConnectionName;
        return result;
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

    uint8_t* buffer = new (std::nothrow) uint8_t[fixedLen + lenName + sizeData];
    if (buffer == nullptr) {
        result.errorCode = Error::NotEnoughMemory;
        return result;
    }
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
    memcpy(buffer + fixedLen, name, lenName);
    if (sizeData > 0) {
		memcpy(buffer + fixedLen+ lenName, data, sizeData);
	}

    result.errorCode = Error::Nil;
    result.data.buffer.reset(buffer);
    result.data.length = fixedLen + lenName + sizeData;
    return result;
}

Result<Array<uint8_t>> Cipher::buildAad(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, const char* name, uint8_t lenName) noexcept {
    Result<Array<uint8_t>> result;
    if (lenName == 0 || lenName > MAX_CONNECTION_NAME_LENGTH) {
        result.errorCode = Error::Message_InvalidConnectionName;
        return result;
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

    uint8_t* buffer = new (std::nothrow) uint8_t[fixedLen + lenName];
    if (buffer == nullptr) {
        result.errorCode = Error::NotEnoughMemory;
        return result;
    }
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
	memcpy(buffer + fixedLen, name, lenName);

    result.errorCode = Error::Nil;
    result.data.buffer.reset(buffer);
    result.data.length = fixedLen + lenName;
    return result;
}

Result<Array<uint8_t>> Cipher::buildCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, const char* name, uint8_t lenName, uint8_t iv[LENGTH_IV], uint8_t* data, uint16_t sizeData, uint8_t authenTag[LENGTH_AUTHEN_TAG]) noexcept {
    return Cipher::buildBytes(msgID, msgTag, msgType, true, isFirst, isLast, isRequest, name, lenName, iv, data, sizeData, authenTag, nullptr);
}

Result<Array<uint8_t>> Cipher::buildNoCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, const char* name, uint8_t lenName, uint8_t* data, uint16_t sizeData, uint8_t sign[LENGTH_SIGN_HMAC]) noexcept {
    return Cipher::buildBytes(msgID, msgTag, msgType, false, isFirst, isLast, isRequest, name, lenName, nullptr, data, sizeData, nullptr, sign);
}

Result<Array<uint8_t>> Cipher::buildBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, const char* name, uint8_t lenName, uint8_t iv[LENGTH_IV], uint8_t* data, uint16_t sizeData, uint8_t authenTag[LENGTH_AUTHEN_TAG], uint8_t sign[LENGTH_SIGN_HMAC]) noexcept {
    Result<Array<uint8_t>> result;
    if (lenName == 0 || lenName > MAX_CONNECTION_NAME_LENGTH) {
        result.errorCode = Error::Message_InvalidConnectionName;
        return result;
	}

    uint8_t lenIV = 0;
    uint8_t lenSign = 0;
    uint8_t lenAuthenTag = 0;
    if (isEncrypted) {
        lenIV = LENGTH_IV;
        lenAuthenTag = LENGTH_AUTHEN_TAG;
    } else {
        lenSign = LENGTH_SIGN_HMAC;
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

    uint16_t lenBuffer = fixedLen + lenAuthenTag + lenIV + lenSign + lenName + sizeData;
    uint8_t* buffer = new (std::nothrow) uint8_t[lenBuffer];
    if (buffer == nullptr) {
        result.errorCode = Error::NotEnoughMemory;
        return result;
    }
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
    uint8_t posData = fixedLen + lenAuthenTag;
	if (isEncrypted) {
		memcpy(buffer + fixedLen, authenTag, lenAuthenTag);
		memcpy(buffer + posData, iv, lenIV);
		posData += lenIV;
	} else {
		memcpy(buffer + fixedLen, sign, lenSign);
		posData += lenSign;
	}
	memcpy(buffer + posData, name, lenName);
	posData += lenName;
	if (sizeData > 0) {
		memcpy(buffer + posData, data, sizeData);
	}

    result.errorCode = Error::Nil;
    result.data.buffer.reset(buffer);
    result.data.length = lenBuffer;
    return result;
}