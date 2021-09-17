#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/error.h>
#include "error/thirdparty.h"
#include "utils/utils_string.h"

std::string Thirdparty::getMbedtlsError(int32_t code) {
    char reason[256];
    mbedtls_strerror(code, reason, 256);
    return UtilsString::format("[MbedTLS]: %s", reason);
}

std::string Thirdparty::getAruduinojsonError(int32_t code) {
    DeserializationError reason((DeserializationError::Code)code);
    return UtilsString::format("[ArduinoJSON]: %s", reason.c_str());
}

std::string Thirdparty::getHttpError(int32_t code) {
    if (code >= 100) {
        return UtilsString::format("[HTTP]: Satus %d", code);
    }
    return UtilsString::format("[HTTP]: %s", HTTPClient::errorToString(code).c_str());
}