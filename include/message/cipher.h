#ifndef _MESSAGE_CIPHER_H_
#define _MESSAGE_CIPHER_H_

#include <memory>
#include <stdint.h>
#include "message/type.h"
#include "message/array.h"
#include "message/define.h"
#include "message/result.h"

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
    uint8_t *data;
    uint8_t lenName;
    char *name;
    MessageType msgType;

    static Result<Array<uint8_t>> buildBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, const char *name, uint8_t lenName, uint8_t iv[LENGTH_IV], uint8_t *data, uint16_t sizeData, uint8_t authenTag[LENGTH_AUTHEN_TAG], uint8_t sign[LENGTH_SIGN_HMAC]);
public:
    Cipher();
    ~Cipher();

    void setMsgID(uint64_t msgID);
    void setMsgTag(uint64_t msgTag);
    void setMsgType(MessageType msgType);
    void setIsFirst(bool isFirst);
    void setIsLast(bool isLast);
    void setIsRequest(bool isRequest);
    void setIsEncrypted(bool isEncrypted);
    void setIV(uint8_t iv[LENGTH_IV]);
    void setSign(uint8_t sign[LENGTH_SIGN_HMAC]);
    void setAuthenTag(uint8_t authenTag[LENGTH_AUTHEN_TAG]);
    void setName(char *name, uint8_t lenName);
    void setData(uint8_t *data, uint16_t sizeData);

    uint64_t getMsgID();
    uint64_t getMsgTag();
    MessageType getMsgType();
    bool getIsFirst();
    bool getIsLast();
    bool getIsRequest();
    bool getIsEncrypted();
    uint8_t *getIV();
    uint8_t *getSign();
    uint8_t *getAuthenTag();
    char *getName();
    uint8_t getLengthName();
    uint8_t *getData(); // Referent to properties
    uint16_t getSizeData();

    Result<Array<uint8_t>> intoBytes();
    Result<Array<uint8_t>> getRawBytes();
    Result<Array<uint8_t>> getAad();

    static Result<std::shared_ptr<Cipher>> parseBytes(uint8_t *buffer, uint16_t sizeBuffer);
    static Result<Array<uint8_t>> buildRawBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, const char *name, uint8_t lenName, uint8_t *data, uint16_t sizeData);
    static Result<Array<uint8_t>> buildAad(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, const char *name, uint8_t lenName);
    static Result<Array<uint8_t>> buildCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, const char *name, uint8_t lenName, uint8_t iv[LENGTH_IV], uint8_t *data, uint16_t sizeData, uint8_t authenTag[LENGTH_AUTHEN_TAG]);
    static Result<Array<uint8_t>> buildNoCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, const char *name, uint8_t lenName, uint8_t *data, uint16_t sizeData, uint8_t sign[LENGTH_SIGN_HMAC]);
};

#endif
