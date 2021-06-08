#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_

#include <cstdint>

#define LENGTH 150

// "ExtTag" has code from 0 to 49
enum ExtTag : uint8_t {
    MbedTLS     = 0U,
    ArduinoJSON = 1U,
    HTTP        = 2U,
    Server      = 3U,
};

class Error {
private:
    static char content[LENGTH];

public:
    enum Code : uint32_t {
        // General has tag 50 = 0x32
        Nil             = 0x32000000,
        NotEnoughMemory = 0x32000001U,

        // CSO_Parser has tag 51 = 0x33
        CSOParser_ValidateHMACFailed = 0x33000001U,

        // CSO_Proxy has tag 52 = 0x34
        CSOProxy_Disconnected  = 0x34000001U,
        CSOProxy_ResponseEmpty = 0x34000002U,

        // CSO_Connection has tag 53 = 0x35
        CSOConnection_Disconnected = 0x35000001U,
        CSOConnection_SetupFailed  = 0x35000002U,

        // CSO_Connector has tag 54 = 0x36
        CSOConnector_NotActivated     = 0x36000001U,
        CSOConnector_MessageQueueFull = 0x36000002U,

        // Message has tag 55 = 0x37
        Message_InvalidBytes          = 0x37000001U,
        Message_InvalidConnectionName = 0x37000002U,

        // Utils has tag 56 = 0x38
        Utils_ConcurrencyQueue_Full  = 0x38000001U,
        Utils_ConcurrencyQueue_Empty = 0x38000002U,

    };

private:
    static bool getContentExtTag(Error::Code code) noexcept;
    static void getContentIntTag(Error::Code code) noexcept;

public:
    static Error::Code adaptExternalCode(ExtTag tag, int32_t code) noexcept;
    static const char* getContent(Error::Code code) noexcept;
};

#endif //_ERROR_CODE_H_