#ifndef _MESSAGE_CIPHER_H_
#define _MESSAGE_CIPHER_H_

#include <stdint.h>
#include <message/define.h>
#include <message/type.h>
#include <message/result.h>

class Cipher {
private:
    uint64_t msgID;
    uint64_t msgTag;
    bool isFirst;
    bool isLast;
    bool isRequest;
    bool isEncrypted;
    uint8_t iv[LENGTH_IV];
    uint8_t sign[LENGTH_SIGN];
    uint8_t authenTag[LENGTH_AUTHEN_TAG];
    uint16_t sizeData;
    uint8_t *data;
    uint8_t lenName;
    char *name;
    MessageType msgType;

    static Result<uint8_t *> buildBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, char *name, uint8_t lenName, uint8_t iv[LENGTH_IV], uint8_t *data, uint16_t sizeData, uint8_t authenTag[LENGTH_AUTHEN_TAG], uint8_t sign[LENGTH_SIGN]);
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
    void setSign(uint8_t sign[LENGTH_SIGN]);
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
    uint8_t *getData();
    uint16_t getSizeData();

    Result<uint8_t *> intoBytes();
    Result<uint8_t *> getRawBytes();
    Result<uint8_t *> getAad();

    static Result<Cipher *> parseBytes(uint8_t *buffer, uint16_t sizeBuffer);
    static Result<uint8_t *> buildRawBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, char *name, uint8_t lenName, uint8_t *data, uint16_t sizeData);
    static Result<uint8_t *> buildAad(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isEncrypted, bool isFirst, bool isLast, bool isRequest, char *name, uint8_t lenName);
    static Result<uint8_t *> buildCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, char *name, uint8_t lenName, uint8_t iv[LENGTH_IV], uint8_t *data, uint16_t sizeData, uint8_t authenTag[LENGTH_AUTHEN_TAG]);
    static Result<uint8_t *> buildNoCipherBytes(uint64_t msgID, uint64_t msgTag, MessageType msgType, bool isFirst, bool isLast, bool isRequest, char *name, uint8_t lenName, uint8_t *data, uint16_t sizeData, uint8_t sign[LENGTH_SIGN]);
};

#endif
