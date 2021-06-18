#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/error.h>
#include "error/error_code.h"

char Error::content[LENGTH];

Error::Code Error::adaptExternalCode(ExternalTag tag, int32_t code) noexcept {
    return Error::Code(abs(code) | (tag << 24U));
}

const char* Error::getContent(Error::Code code) noexcept {
    if (!Error::getContentExternalTag(code)) {
        Error::getContentInternalTag(code);
    }
    return Error::content;
}

bool Error::getContentExternalTag(Error::Code code) noexcept {
    ExternalTag tag = (ExternalTag)(code >> 24U);
    if (tag == ExternalTag::MbedTLS) {
        memcpy(Error::content, "[MbedTLS] ", 10);
        mbedtls_strerror(-(code & 0x0000FFFFU), Error::content + 10, LENGTH - 10);
        return true;
    }

    if (tag == ExternalTag::ArduinoJSON) {
        DeserializationError oriCode((DeserializationError::Code)(code & 0x0000FFFFU));
        sprintf(Error::content, "[ArduinoJSON] %s", oriCode.c_str());
        return true;
    }

    if (tag == ExternalTag::HTTP) {
        int32_t oriCode = (code & 0x0000FFFFU);
        if (oriCode >= 100) {
            sprintf(Error::content, "[HTTP] Satus: %d", oriCode);
            return true;
        }

        sprintf(Error::content, "[HTTP] %s", HTTPClient::errorToString(-oriCode).c_str());
        return true;
    }

    if (tag == ExternalTag::Server) {
        sprintf(Error::content, "[Server] Error code: %d", (code & 0x0000FFFFU));
        return true;
    }
    return false;
}

void Error::getContentInternalTag(Error::Code code) noexcept {
    //========
    // General
    //========
    if (code == Error::NotEnoughMemory) {
        strcpy(Error::content, "[General] Not enough memory for allocating object or array");
        return;
    }

    //===========
    // CSO_Parser
    //===========
    if (code == Error::CSOParser_ValidateHMACFailed) {
        strcpy(Error::content, "[CSO_Parser] Validate HMAC failed");
        return;
    }

    //==========
    // CSO_Proxy
    //==========
    if (code == Error::CSOProxy_Disconnected) {
        strcpy(Error::content, "[CSO_Proxy] HTTP can not connect with server");
        return;
    }
    if (code == Error::CSOProxy_ResponseEmpty) {
        strcpy(Error::content, "[CSO_Proxy] Response is empty");
        return;
    }
    if (code == Error::CSOProxy_InvalidHubAddress) {
        strcpy(Error::content, "[CSO_Proxy] Invalid hub address");
    }
    
    //===============
    // CSO_Connection
    //===============
    if (code == Error::CSOConnection_Disconnected) {
        strcpy(Error::content, "[CSO_Connection] Socket can not connect with server");
        return;
    }
    if (code == Error::CSOConnection_SetupFailed) {
        strcpy(Error::content, "[CSO_Connection] Setup connection failed");
        return;
    }

    //==============
    // CSO_Connector
    //==============
    if (code == Error::CSOConnector_NotActivated) {
        strcpy(Error::content, "[CSO_Connector] System is not activated");
        return;
    }
    if (code == Error::CSOConnector_MessageQueueFull) {
        strcpy(Error::content, "[CSO_Connector] Message queue is full");
        return;
    }

    //========
    // Message
    //========
    if (code == Error::Message_InvalidBytes) {
        strcpy(Error::content, "[Message] Invalid bytes");
        return;
    }
    if (code == Error::Message_InvalidConnectionName) {
        strcpy(Error::content, "[Message] Invalid connection name");
        return;
    }
    
    //================
    // Synchronization
    //================
    if (code == Error::Synchronization_ConcurrencyQueue_Full) {
        strcpy(Error::content, "[Synchronization_ConcurrencyQueue] Queue is full");
        return;
    }
    if (code == Error::Synchronization_ConcurrencyQueue_Empty) {
        strcpy(Error::content, "[Synchronization_ConcurrencyQueue] Queue is empty");
        return;
    }

    //=============
    // Code invalid
    //=============
    sprintf(Error::content, "[UNKNOWN] Tag:%d, error-code:%d", (code >> 24U), (code & 0x0000FFFFU));
}