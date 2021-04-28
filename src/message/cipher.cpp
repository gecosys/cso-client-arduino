#include <message/cipher.h>
#include <message/define.h>

#include <string.h>

#define MAX_CONNECTION_NAME_LENGTH 36

Cipher::Cipher() {
    this->name = nullptr;
    this->data = nullptr;
}

Cipher::~Cipher() {
    if (this->name != nullptr) {
        delete []this->name;
    }
    if (this->data != nullptr) {
        delete []this->data;
    }
}

void Cipher::setMsgID(uint64_t msgID) {
    this->msgID = msgID;
}

void Cipher::setMsgTag(uint64_t msgTag) {
    this->msgTag = msgTag;
}

void Cipher::setMsgType(MessageType msgType) {
    this->msgType = msgType;
}

void Cipher::setIsFirst(bool isFirst) {
    this->isFirst = isFirst;
}

void Cipher::setIsLast(bool isLast) {
    this->isLast = isLast;
}

void Cipher::setIsRequest(bool isRequest) {
    this->isRequest = isRequest;
}

void Cipher::setIsEncrypted(bool isEncrypted) {
    this->isEncrypted = isEncrypted;
}

void Cipher::setIV(uint8_t iv[LENGTH_IV]) {
    memcpy(this->iv, iv, LENGTH_IV);
}

void Cipher::setSign(uint8_t sign[LENGTH_SIGN]) {
    memcpy(this->sign, sign, LENGTH_SIGN);
}

void Cipher::setAuthenTag(uint8_t authenTag[LENGTH_AUTHEN_TAG]) {
    memcpy(this->authenTag, authenTag, LENGTH_AUTHEN_TAG);
}

void Cipher::setName(char *name, uint8_t lenName) {
    this->lenName = lenName;
    if (this->name != nullptr) {
        delete []this->name;
    }
    this->name = name;
}

void Cipher::setData(uint8_t *data, uint16_t sizeData) {
    this->sizeData = sizeData;
    if (this->data != nullptr) {
        delete []this->data;
    }
    this->data = data;
}


uint64_t Cipher::getMsgID() {
    return this->msgID;
}

uint64_t Cipher::getMsgTag() {
    return this->msgTag;
}

MessageType Cipher::getMsgType() {
    return this->msgType;
}

bool Cipher::getIsFirst() {
    return this->isFirst;
}

bool Cipher::getIsLast() {
    return this->isLast;
}

bool Cipher::getIsRequest() {
    return this->isRequest;
}

bool Cipher::getIsEncrypted() {
    return this->isEncrypted;
}

uint8_t *Cipher::getIV() {
    return this->iv;
}

uint8_t *Cipher::getSign() {
    return this->sign;
}

uint8_t *Cipher::getAuthenTag() {
    return this->authenTag;
}

char *Cipher::getName() {
    return this->name;
}

uint8_t Cipher::getLengthName() {
    return this->lenName;
}

uint8_t *Cipher::getData() {
    return this->data;
}

uint16_t Cipher::getSizeData() {
    return this->sizeData;
}

Result<uint8_t *> Cipher::intoBytes() {
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

Result<uint8_t *> Cipher::getRawBytes() {
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

Result<uint8_t *> Cipher::getAad() {
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
Result<Cipher *> Cipher::parseBytes(uint8_t *buffer, uint16_t sizeBuffer) {
    Result<Cipher *> result;
    uint8_t fixedLen = 10;
    uint8_t posAuthenTag = 10;
    if (sizeBuffer < fixedLen) {
        result.errorCode = ERROR_CODE_INVALID_BYTES;
        return result;
    }

    uint8_t flag = buffer[8];
    bool isEncrypted = (flag & 0x80) != 0;
    uint64_t msgID = ((uint64_t)buffer[7] << 56) | ((uint64_t)buffer[6] << 48) | ((uint64_t)buffer[5] << 40) | ((uint64_t)buffer[4] << 32) | ((uint64_t)buffer[3] << 24) | ((uint64_t)buffer[2] << 16) | ((uint64_t)buffer[1] << 8) | (uint64_t)buffer[0];

    uint8_t lenName = buffer[9];
    uint64_t msgTag = 0;
    if ((flag & 0x08) != 0) {
        fixedLen += 8;
        posAuthenTag += 8;
        if (sizeBuffer < fixedLen) {
            result.errorCode = ERROR_CODE_INVALID_BYTES;
            return result;
        }
        msgTag = ((uint64_t)buffer[17] << 56) | ((uint64_t)buffer[16] << 48) | ((uint64_t)buffer[15] << 40) | ((uint64_t)buffer[14] << 32) | ((uint64_t)buffer[13] << 24) | ((uint64_t)buffer[12] << 16) | ((uint64_t)buffer[11] << 8) | (uint64_t)buffer[10];
    }

    if (isEncrypted) {
        fixedLen += LENGTH_AUTHEN_TAG + LENGTH_IV;
    }
    if (lenName == 0 || lenName > MAX_CONNECTION_NAME_LENGTH) {
		result.errorCode = ERROR_CODE_INVALID_CONNECTION_NAME;
        return result;
	}
    if (sizeBuffer < fixedLen+lenName) {
		result.errorCode = ERROR_CODE_INVALID_BYTES;
        return result;
	}

    // Parse AUTHEN_TAG, IV
    uint8_t iv[LENGTH_IV];
    uint8_t sign[LENGTH_SIGN];
    uint8_t authenTag[LENGTH_AUTHEN_TAG];
    if (isEncrypted) {
        uint8_t posIV = posAuthenTag + LENGTH_AUTHEN_TAG;
        memcpy(authenTag, buffer+posAuthenTag, LENGTH_AUTHEN_TAG);
		memcpy(iv, buffer+posIV, LENGTH_IV);
    } else {
        uint8_t posSign = fixedLen;
        fixedLen += LENGTH_SIGN;
        if (sizeBuffer < fixedLen+lenName) {
            result.errorCode = ERROR_CODE_INVALID_BYTES;
            return result;
        }
        memcpy(sign, buffer+posSign, LENGTH_SIGN);
    }

    // Parse name
    uint8_t posData = fixedLen + lenName;
    char *name = new char[lenName+1];
    name[lenName] = '\0';
    memcpy(name, buffer+fixedLen, lenName);

    // Parse data
    uint8_t *data = nullptr;
    uint16_t sizeData = sizeBuffer - posData;
    if (sizeData > 0) {
		data = new uint8_t[sizeData];
		memcpy(data, buffer+posData, sizeData);
	}

    Cipher *cipher = new Cipher();
    cipher->msgID = msgID;
    cipher->msgTag = msgTag;
    cipher->msgType = (MessageType)(flag & 0x07);
    cipher->isFirst = (flag & 0x40) != 0;
    cipher->isLast = (flag & 0x20) != 0;
    cipher->isRequest = (flag & 0x10) != 0;
    cipher->isEncrypted = isEncrypted;
    memcpy(cipher->iv, iv, LENGTH_IV);
    memcpy(cipher->sign, sign, LENGTH_SIGN);
    memcpy(cipher->authenTag, authenTag, LENGTH_AUTHEN_TAG);
    cipher->sizeData = sizeData;
    cipher->data = data;
    cipher->lenName = lenName;
    cipher->name = name;

    result.data = cipher;
    result.errorCode = SUCCESS;
    return result;
}

Result<uint8_t *> Cipher::buildRawBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, char *name, uint8_t lenName, uint8_t *data, uint16_t sizeData) {
    Result<uint8_t *> result;
    if (lenName == 0 || lenName > MAX_CONNECTION_NAME_LENGTH) {
		result.errorCode = ERROR_CODE_INVALID_CONNECTION_NAME;
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

    uint8_t *buffer = new uint8_t[fixedLen+lenName+sizeData];
    buffer[0] = (uint8_t)msgID;
	buffer[1] = (uint8_t)(msgID >> 8);
	buffer[2] = (uint8_t)(msgID >> 16);
	buffer[3] = (uint8_t)(msgID >> 24);
	buffer[4] = (uint8_t)(msgID >> 32);
	buffer[5] = (uint8_t)(msgID >> 40);
	buffer[6] = (uint8_t)(msgID >> 48);
	buffer[7] = (uint8_t)(msgID >> 56);
	buffer[8] = (uint8_t)(bEncrypted<<7 | bFirst<<6 | bLast<<5 | bRequest<<4 | bUseTag<<3 | uint8_t(msgType));
	buffer[9] = lenName;
    if (msgTag > 0) {
        buffer[10] = (uint8_t)msgTag;
		buffer[11] = (uint8_t)(msgTag >> 8);
		buffer[12] = (uint8_t)(msgTag >> 16);
		buffer[13] = (uint8_t)(msgTag >> 24);
		buffer[14] = (uint8_t)(msgTag >> 32);
		buffer[15] = (uint8_t)(msgTag >> 40);
		buffer[16] = (uint8_t)(msgTag >> 48);
		buffer[17] = (uint8_t)(msgTag >> 56);
    }
    memcpy(buffer+fixedLen, name, lenName);
    if (sizeData > 0) {
		memcpy(buffer+(fixedLen+lenName), data, sizeData);
	}

    result.errorCode = SUCCESS;
    result.data = buffer;
    return result;
}

Result<uint8_t *> Cipher::buildAad(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, char *name, uint8_t lenName) {
    Result<uint8_t *> result;
    if (lenName == 0 || lenName > MAX_CONNECTION_NAME_LENGTH) {
        result.errorCode = ERROR_CODE_INVALID_CONNECTION_NAME;
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

    uint8_t *buffer = new uint8_t [fixedLen+lenName];
	buffer[0] = (uint8_t)msgID;
	buffer[1] = (uint8_t)(msgID >> 8);
	buffer[2] = (uint8_t)(msgID >> 16);
	buffer[3] = (uint8_t)(msgID >> 24);
	buffer[4] = (uint8_t)(msgID >> 32);
	buffer[5] = (uint8_t)(msgID >> 40);
	buffer[6] = (uint8_t)(msgID >> 48);
	buffer[7] = (uint8_t)(msgID >> 56);
	buffer[8] = (uint8_t)(bEncrypted<<7 | bFirst<<6 | bLast<<5 | bRequest<<4 | bUseTag<<3 | (uint8_t)msgType);
	buffer[9] = lenName;
	if (msgTag > 0) {
		buffer[10] = (uint8_t)msgTag;
		buffer[11] = (uint8_t)(msgTag >> 8);
		buffer[12] = (uint8_t)(msgTag >> 16);
		buffer[13] = (uint8_t)(msgTag >> 24);
		buffer[14] = (uint8_t)(msgTag >> 32);
		buffer[15] = (uint8_t)(msgTag >> 40);
		buffer[16] = (uint8_t)(msgTag >> 48);
		buffer[17] = (uint8_t)(msgTag >> 56);
	}
	memcpy(buffer+fixedLen, name, lenName);

    result.errorCode = SUCCESS;
    result.data = buffer;
    return result;
}

Result<uint8_t *> Cipher::buildCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, char *name, uint8_t lenName, uint8_t iv[LENGTH_IV], uint8_t *data, uint16_t sizeData, uint8_t authenTag[LENGTH_AUTHEN_TAG]) {
    return Cipher::buildBytes(msgID, msgTag, msgType, true, isFirst, isLast, isRequest, name, lenName, iv, data, sizeData, authenTag, nullptr);
}

Result<uint8_t *> Cipher::buildNoCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, char *name, uint8_t lenName, uint8_t *data, uint16_t sizeData, uint8_t sign[LENGTH_SIGN]) {
    return Cipher::buildBytes(msgID, msgTag, msgType, false, isFirst, isLast, isRequest, name, lenName, nullptr, data, sizeData, nullptr, sign);
}

Result<uint8_t *> Cipher::buildBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, char *name, uint8_t lenName, uint8_t iv[LENGTH_IV], uint8_t *data, uint16_t sizeData, uint8_t authenTag[LENGTH_AUTHEN_TAG], uint8_t sign[LENGTH_SIGN]) {
    Result<uint8_t *> result;
    if (lenName == 0 || lenName > MAX_CONNECTION_NAME_LENGTH) {
        result.errorCode = ERROR_CODE_INVALID_CONNECTION_NAME;
        return result;
	}

    uint8_t lenIV = 0;
    uint8_t lenSign = 0;
    uint8_t lenAuthenTag = 0;
    if (isEncrypted) {
        lenIV = LENGTH_IV;
        lenAuthenTag = LENGTH_AUTHEN_TAG;
    } else {
        lenSign = LENGTH_SIGN;
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
    uint8_t *buffer = new uint8_t[lenBuffer];
    buffer[0] = (uint8_t)msgID;
	buffer[1] = (uint8_t)(msgID >> 8);
	buffer[2] = (uint8_t)(msgID >> 16);
	buffer[3] = (uint8_t)(msgID >> 24);
	buffer[4] = (uint8_t)(msgID >> 32);
	buffer[5] = (uint8_t)(msgID >> 40);
	buffer[6] = (uint8_t)(msgID >> 48);
	buffer[7] = (uint8_t)(msgID >> 56);
	buffer[8] = (uint8_t)(bEncrypted<<7 | bFirst<<6 | bLast<<5 | bRequest<<4 | bUseTag<<3 | (uint8_t)msgType);
	buffer[9] = lenName;
	if (msgTag > 0) {
		buffer[10] = (uint8_t)msgTag;
		buffer[11] = (uint8_t)(msgTag >> 8);
		buffer[12] = (uint8_t)(msgTag >> 16);
		buffer[13] = (uint8_t)(msgTag >> 24);
		buffer[14] = (uint8_t)(msgTag >> 32);
		buffer[15] = (uint8_t)(msgTag >> 40);
		buffer[16] = (uint8_t)(msgTag >> 48);
		buffer[17] = (uint8_t)(msgTag >> 56);
	}
    uint8_t posData = fixedLen + lenAuthenTag;
	if (isEncrypted) {
		memcpy(buffer+fixedLen, authenTag, lenAuthenTag);
		memcpy(buffer+posData, iv, lenIV);
		posData += lenIV;
	} else {
		memcpy(buffer+fixedLen, sign, lenSign);
		posData += lenSign;
	}
	memcpy(buffer+posData, name, lenName);
	posData += lenName;
	if (sizeData > 0) {
		memcpy(buffer+posData, data, sizeData);
	}

    result.errorCode = SUCCESS;
    result.data = buffer;
    return result;
}