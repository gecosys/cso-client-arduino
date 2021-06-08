#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/error.h>
#include "error/error_code.h"

char Error::content[LENGTH];

Error::Code Error::adaptExternalCode(ExtTag tag, int32_t code) noexcept {
    return Error::Code(abs(code) | (tag & 0xFF000000U));
}

const char* Error::getContent(Error::Code code) noexcept {
    if (!Error::getContentExtTag(code)) {
        Error::getContentIntTag(code);
    }
    return Error::content;
}

bool Error::getContentExtTag(Error::Code code) noexcept {
    ExtTag tag = (ExtTag)(code >> 24U);
    if (tag == ExtTag::MbedTLS) {
        memcpy(Error::content, "[MbedTLS]", 9);
        mbedtls_strerror(-(code & 0x00FFFFFFU), Error::content + 9, LENGTH - 9);
        return true;
    }

    if (tag == ExtTag::ArduinoJSON) {
        DeserializationError oriCode((DeserializationError::Code)(code & 0x00FFFFFFU));
        sprintf(Error::content, "[ArduinoJSON]%s", oriCode.c_str());
        return true;
    }

    if (tag == ExtTag::HTTP) {
        int32_t oriCode = (code & 0x00FFFFFFU);
        if (oriCode >= 100) {
            sprintf(Error::content, "[HTTP]Satus: %d", oriCode);
            return true;
        }

        sprintf(Error::content, "[HTTP]%s", HTTPClient::errorToString(-oriCode).c_str());
        return true;
    }

    if (tag == ExtTag::Server) {
        sprintf(Error::content, "[Server]Error code: %d", (code & 0x00FFFFFFU));
        return true;
    }
    return false;
}

void Error::getContentIntTag(Error::Code code) noexcept {
    //========
    // General
    //========
    if (code == Error::NotEnoughMemory) {
        strcpy(Error::content, "[General]Not enough memory for allocating object or array");
        return;
    }

    //===========
    // CSO_Parser
    //===========
    if (code == Error::CSOParser_ValidateHMACFailed) {
        strcpy(Error::content, "[CSO_Parser]Validate HMAC failed");
        return;
    }

    //==========
    // CSO_Proxy
    //==========
    if (code == Error::CSOProxy_Disconnected) {
        strcpy(Error::content, "[CSO_Proxy]HTTP can not connect with server");
        return;
    }
    if (code == Error::CSOProxy_ResponseEmpty) {
        strcpy(Error::content, "[CSO_Proxy]Response is empty");
        return;
    }
    
    //===============
    // CSO_Connection
    //===============
    if (code == Error::CSOConnection_Disconnected) {
        strcpy(Error::content, "[CSO_Connection]Socket can not connect with server");
        return;
    }
    if (code == Error::CSOConnection_SetupFailed) {
        strcpy(Error::content, "[CSO_Connection]Setup connection failed");
        return;
    }

    //==============
    // CSO_Connector
    //==============
    if (code == Error::CSOConnector_NotActivated) {
        strcpy(Error::content, "[CSO_Connector]System is not activated");
        return;
    }
    if (code == Error::CSOConnector_MessageQueueFull) {
        strcpy(Error::content, "[CSO_Connector]Message queue is full");
        return;
    }

    //========
    // Message
    //========
    if (code == Error::Message_InvalidBytes) {
        strcpy(Error::content, "[Message]Invalid bytes");
        return;
    }
    if (code == Error::Message_InvalidConnectionName) {
        strcpy(Error::content, "[Message]Invalid connection name");
        return;
    }

    //======
    // Utils
    //======
    if (code == Error::Utils_ConcurrencyQueue_Full) {
        strcpy(Error::content, "[Utils_ConcurrencyQueue]Queue is full");
        return;
    }
    if (code == Error::Utils_ConcurrencyQueue_Empty) {
        strcpy(Error::content, "[Utils_ConcurrencyQueue]Queue is empty");
        return;
    }

    //=============
    // Code invalid
    //=============
    sprintf(Error::content, "[UNKNOWN]Tag:%d, error-code:%d", (code >> 24U), (code & 0x00FFFFFFU));
}