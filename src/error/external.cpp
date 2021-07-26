#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/error.h>
#include "error/external.h"
#include "utils/utils_string.h"

std::string External::getReason(uint8_t extID, int32_t reasonCode) {
    //========
    // MbedTLS
    //========
    if (extID == External::ID::MbedTLS) {
        char reason[256];
        mbedtls_strerror(-reasonCode, reason, 256);
        return UtilsString::format("[MbedTLS]: %s", reason);
    }

    //============
    // ArduinoJSON
    //============
    if (extID == External::ID::ArduinoJSON) {
        DeserializationError reason((DeserializationError::Code)reasonCode);
        return UtilsString::format("[ArduinoJSON]: %s", reason.c_str());
    }

    //=====
    // HTTP
    //=====
    if (extID == External::ID::HTTP) {
        if (reasonCode >= 100) {
            return UtilsString::format("[HTTP]: Satus %d", reasonCode);
        }
        return UtilsString::format("[HTTP]: %s", HTTPClient::errorToString(-reasonCode).c_str());
    }

    //=======
    // Server
    //=======
    if (extID == External::ID::Server) {
        return UtilsString::format("[Server]: Error code %d", reasonCode);
    }

    //========
    // Unknown
    //========
    return "UNKNOWN";
}