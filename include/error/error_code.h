#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_

#include <cstdint>

// The exception handling:
//      1. Just throw an exception in constructors and some operators if it is really necessary.
//         Return error codes in other methods.
//      2. Should throw a string instead of a std::exception 
//         because a std:exception can throw another unexpected error especially from its copy constructors.
//      3. The thrown string should include the error location (class, function) and 
//         the error reason for easy tracking
// The reason for doing this is that throwing exceptions is discouraged especially in embedded systems.

#define LENGTH 150

// "ExternalTag" has tag from 0 to 49
// "InternalTag" has tag is 255
enum ExternalTag : uint8_t {
    MbedTLS     = 0U,
    ArduinoJSON = 1U,
    HTTP        = 2U,
    Server      = 3U,
};

enum Location : uint8_t {
    CSO_Proxy    = 1U,
    Utils_BigNum = 2U,
    Utils_AES    = 3U,
    Utils_HMAC   = 4U,
    Utils_RSA    = 5U,
};

class Error {
private:
    static char content[LENGTH];

public:
    enum Code : uint32_t {
        // General has a code range from 0 to 20
        Nil             = 0xFF000000U,
        NotEnoughMemory = 0xFF000001U,

        // CSO_Parser has a code range from 21 to 30
        CSOParser_ValidateHMACFailed = 0xFF000015U,

        // CSO_Proxy has a code range from 31 to 40
        CSOProxy_Disconnected       = 0xFF00001FU,
        CSOProxy_ResponseEmpty      = 0xFF000020U,
        CSOProxy_InvalidHubAddress  = 0xFF000021U,

        // CSO_Connection has a code range from 41 to 50
        CSOConnection_Disconnected = 0xFF000029U,
        CSOConnection_SetupFailed  = 0xFF00002AU,

        // CSO_Connector has a code range from 51 to 60
        CSOConnector_NotActivated     = 0xFF000033U,
        CSOConnector_MessageQueueFull = 0xFF000034U,

        // Message has a code range from 61 to 70
        Message_InvalidBytes          = 0xFF00003DU,
        Message_InvalidConnectionName = 0xFF00003EU,

        // Synchronization has a code range from 71 to 80
        Synchronization_ConcurrencyQueue_Full  = 0xFF000047U,
        Synchronization_ConcurrencyQueue_Empty = 0xFF000048U,
    };

private:
    static const char* getContentLocation(Error::Code code) noexcept;
    static bool getContentExternalTag(Error::Code code) noexcept;
    static void getContentInternalTag(Error::Code code) noexcept;

public:
    static Error::Code adaptExternalCode(Location location, ExternalTag tag, int32_t code) noexcept;
    static const char* getContent(Error::Code code) noexcept;
};

#endif //_ERROR_CODE_H_