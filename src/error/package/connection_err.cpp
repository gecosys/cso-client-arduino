#include "error/external.h"
#include "error/package/connection_err.h"
#include "utils/utils_string.h"

const uint8_t ConnectionErr::ID = 1U;

const std::string ConnectionErr::reasonText[CONNECTION_ERR_NREASON] = {
    "Socket connect failed",
    "Socket is disconnected",
};

const std::string ConnectionErr::funcName[CONNECTION_ERR_NFUNC] = {
    "Connection::connect",
    "Connection::loopListen",
    "Connection::sendMessage",
    "Connection::setup",
};

const std::string& ConnectionErr::getReason(uint8_t reasonCode) noexcept {
    if (reasonCode > CONNECTION_ERR_NREASON) {
        return "UNKNOWN";
    }
    return ConnectionErr::reasonText[reasonCode - 1];
}

const std::string& ConnectionErr::getFunctionName(uint8_t funcID) noexcept {
    if (funcID > CONNECTION_ERR_NFUNC) {
        return "UNKNOWN";
    }
    return ConnectionErr::funcName[funcID - 1];
}

std::string ConnectionErr::getString(uint8_t funcID, int32_t reasonCode, uint8_t extID) {
    auto funcName = ConnectionErr::getFunctionName(funcID);
    if (extID == External::Nil) {
        return UtilsString::format("[%s]:%s", funcName.c_str(), ConnectionErr::getReason(reasonCode).c_str());
    }
    return UtilsString::format("[%s]:%s", funcName.c_str(), External::getReason(extID, reasonCode).c_str());
}