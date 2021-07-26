#include "error/external.h"
#include "error/package/connector_err.h"
#include "utils/utils_string.h"

const uint8_t ConnectorErr::ID = 2U;

const std::string ConnectorErr::reasonText[CONNECTOR_ERR_NREASON] = {
    "Connection is not activated",
    "Message queue is full",
};

const std::string ConnectorErr::funcName[CONNECTOR_ERR_NFUNC] = {
    "Connector::sendMessage",
    "Connector::sendGroupMessage",
    "Connector::sendMessageAndRetry",
    "Connector::sendGroupMessageAndRetry",
};

const std::string& ConnectorErr::getReason(uint8_t reasonCode) noexcept {
    if (reasonCode > CONNECTOR_ERR_NREASON) {
        return "UNKNOWN";
    }
    return ConnectorErr::reasonText[reasonCode - 1];
}

const std::string& ConnectorErr::getFunctionName(uint8_t funcID) noexcept {
    if (funcID > CONNECTOR_ERR_NFUNC) {
        return "UNKNOWN";
    }
    return ConnectorErr::funcName[funcID - 1];
}

std::string ConnectorErr::getString(uint8_t funcID, int32_t reasonCode, uint8_t extID) {
    auto funcName = ConnectorErr::getFunctionName(funcID);
    if (extID == External::Nil) {
        return UtilsString::format("[%s]:%s", funcName.c_str(), ConnectorErr::getReason(reasonCode).c_str());
    }
    return UtilsString::format("[%s]:%s", funcName.c_str(), External::getReason(extID, reasonCode).c_str());
}