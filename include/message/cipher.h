#ifndef _MESSAGE_CIPHER_H_
#define _MESSAGE_CIPHER_H_

#include <memory>
#include <cstdint>
#include "utils/array.h"
#include "utils/result.h"
#include "message/type.h"
#include "message/define.h"

class Cipher {
private:
    uint64_t msgID;
    uint64_t msgTag;
    bool isFirst;
    bool isLast;
    bool isRequest;
    bool isEncrypted;
    uint8_t iv[LENGTH_IV];
    uint8_t sign[LENGTH_SIGN_HMAC];
    uint8_t authenTag[LENGTH_AUTHEN_TAG];
    uint16_t sizeData;
    uint8_t* data;
    uint8_t lenName;
    char* name;
    MessageType msgType;

    static Result<Array<uint8_t>> buildBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, const char* name, uint8_t lenName, uint8_t iv[LENGTH_IV], uint8_t* data, uint16_t sizeData, uint8_t authenTag[LENGTH_AUTHEN_TAG], uint8_t sign[LENGTH_SIGN_HMAC]) noexcept;

public:
    Cipher() noexcept;
    Cipher(Cipher&& other) noexcept;
    Cipher(const Cipher& other) = delete;
    ~Cipher() noexcept;

    Cipher& operator=(const Cipher& other) = delete;
    Cipher& operator=(Cipher&& other) noexcept;

    void setMsgID(uint64_t msgID) noexcept;
    void setMsgTag(uint64_t msgTag) noexcept;
    void setMsgType(MessageType msgType) noexcept;
    void setIsFirst(bool isFirst) noexcept;
    void setIsLast(bool isLast) noexcept;
    void setIsRequest(bool isRequest) noexcept;
    void setIsEncrypted(bool isEncrypted) noexcept;
    void setIV(uint8_t iv[LENGTH_IV]) noexcept;
    void setSign(uint8_t sign[LENGTH_SIGN_HMAC]) noexcept;
    void setAuthenTag(uint8_t authenTag[LENGTH_AUTHEN_TAG]) noexcept;
    void setName(char* name, uint8_t lenName) noexcept;
    void setData(uint8_t* data, uint16_t sizeData) noexcept;

    uint64_t getMsgID() noexcept;
    uint64_t getMsgTag() noexcept;
    MessageType getMsgType() noexcept;
    bool getIsFirst() noexcept;
    bool getIsLast() noexcept;
    bool getIsRequest() noexcept;
    bool getIsEncrypted() noexcept;
    uint8_t* getIV() noexcept;
    uint8_t* getSign() noexcept;
    uint8_t* getAuthenTag() noexcept;
    char* getName() noexcept;
    uint8_t getLengthName() noexcept;
    uint8_t* getData() noexcept;
    uint16_t getSizeData() noexcept;

    Result<Array<uint8_t>> intoBytes() noexcept;
    Result<Array<uint8_t>> getRawBytes() noexcept;
    Result<Array<uint8_t>> getAad() noexcept;

    static Result<std::unique_ptr<Cipher>> parseBytes(uint8_t* buffer, uint16_t sizeBuffer) noexcept;
    static Result<Array<uint8_t>> buildRawBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, const char* name, uint8_t lenName, uint8_t* data, uint16_t sizeData) noexcept;
    static Result<Array<uint8_t>> buildAad(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, const char* name, uint8_t lenName) noexcept;
    static Result<Array<uint8_t>> buildCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, const char* name, uint8_t lenName, uint8_t iv[LENGTH_IV], uint8_t* data, uint16_t sizeData, uint8_t authenTag[LENGTH_AUTHEN_TAG]) noexcept;
    static Result<Array<uint8_t>> buildNoCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, const char* name, uint8_t lenName, uint8_t* data, uint16_t sizeData, uint8_t sign[LENGTH_SIGN_HMAC]) noexcept;
};

#endif
