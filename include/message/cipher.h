#ifndef MESSAGE_CIPHER_H
#define MESSAGE_CIPHER_H

#include <tuple>
#include <string>
#include <memory>
#include "error/error.h"
#include "entity/array.h"
#include "message/type.h"

class Cipher {
private:
    uint64_t msgID;
    uint64_t msgTag;
    bool isFirst;
    bool isLast;
    bool isRequest;
    bool isEncrypted;
    Array<uint8_t> iv;
    Array<uint8_t> authenTag;
    Array<uint8_t> sign;
    Array<uint8_t> data;
    std::string name;
    MessageType msgType;

private:
    static std::tuple<Error::Code, Array<uint8_t>> buildBytes(
        uint64_t msgID,
        uint64_t msgTag,
        MessageType msgType,
        bool isEncrypted,
        bool isFirst,
        bool isLast,
        bool isRequest,
        const std::string& name,
        const Array<uint8_t>& iv,
        const Array<uint8_t>& authenTag,
        const Array<uint8_t>& sign,
        const Array<uint8_t>& data);

public:
    Cipher() noexcept;
    Cipher(const Cipher& other) = default;
    Cipher(Cipher&& other) noexcept;
    ~Cipher() noexcept = default;

    Cipher& operator=(const Cipher& other) = default;
    Cipher& operator=(Cipher&& other) noexcept;

    void setMsgID(uint64_t msgID) noexcept;
    void setMsgTag(uint64_t msgTag) noexcept;
    void setMsgType(MessageType msgType) noexcept;
    void setIsFirst(bool isFirst) noexcept;
    void setIsLast(bool isLast) noexcept;
    void setIsRequest(bool isRequest) noexcept;
    void setIsEncrypted(bool isEncrypted) noexcept;
    // "iv" has fixed length is LENGTH_IV in message/define.h
    Error::Code setIV(const Array<uint8_t>& iv) noexcept;
    // "authenTag" has fixed length is LENGTH_AUTHEN_TAG in message/define.h
    Error::Code setAuthenTag(const Array<uint8_t>& authenTag) noexcept;
    // "sign" has fixed length is LENGTH_SIGN in message/define.h
    Error::Code setSign(const Array<uint8_t>& sign) noexcept;
    void setName(const std::string& name);
    void setName(std::string&& name) noexcept;
    void setData(const Array<uint8_t>& data);
    void setData(Array<uint8_t>&& data) noexcept;

    uint64_t getMsgID() const noexcept;
    uint64_t getMsgTag() const noexcept;
    MessageType getMsgType() const noexcept;
    bool getIsFirst() const noexcept;
    bool getIsLast() const noexcept;
    bool getIsRequest() const noexcept;
    bool getIsEncrypted() const noexcept;

    // 'iv' has fixed length is 'LENGTH_IV' in' message/define.h'
    const Array<uint8_t>& getIV() const noexcept;
    // 'authenTag' has fixed length is 'LENGTH_AUTHEN_TAG' in 'message/define.h'
    const Array<uint8_t>& getAuthenTag() const noexcept;
    // 'sign' has fixed length is 'LENGTH_SIGN' in 'message/define.h'
    const Array<uint8_t>& getSign() const noexcept;
    const std::string& getName() const noexcept;
    const Array<uint8_t>& getData() const noexcept;

    std::tuple<Error::Code, Array<uint8_t>> intoBytes();
    std::tuple<Error::Code, Array<uint8_t>> getRawBytes();
    std::tuple<Error::Code, Array<uint8_t>> getAad();

    static std::tuple<Error::Code, std::unique_ptr<Cipher>> parseBytes(const Array<uint8_t>& buffer);

    static std::tuple<Error::Code, Array<uint8_t>> buildRawBytes(
        uint64_t msgID,
        uint64_t msgTag,
        MessageType msgType,
        bool isEncrypted,
        bool isFirst,
        bool isLast,
        bool isRequest,
        const std::string& name,
        const Array<uint8_t>& data);

    static std::tuple<Error::Code, Array<uint8_t>> buildAad(
        uint64_t msgID,
        uint64_t msgTag,
        MessageType msgType,
        bool isEncrypted,
        bool isFirst,
        bool isLast,
        bool isRequest,
        const std::string& name);

    // "iv" has fixed length is 12
    // "authenTag" has fixed length is 16
    static std::tuple<Error::Code, Array<uint8_t>> buildCipherBytes(
        uint64_t msgID,
        uint64_t msgTag,
        MessageType msgType,
        bool isFirst,
        bool isLast,
        bool isRequest,
        const std::string& name,
        const Array<uint8_t>& iv,
        const Array<uint8_t>& authenTag,
        const Array<uint8_t>& data);

    static std::tuple<Error::Code, Array<uint8_t>> buildNoCipherBytes(
        uint64_t msgID,
        uint64_t msgTag,
        MessageType msgType,
        bool isFirst,
        bool isLast,
        bool isRequest,
        const std::string& name,
        const Array<uint8_t>& sign,
        const Array<uint8_t>& data);
};

#endif // !MESSAGE_CIPHER_H