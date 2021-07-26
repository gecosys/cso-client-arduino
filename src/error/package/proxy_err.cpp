#include "error/external.h"
#include "error/package/proxy_err.h"
#include "utils/utils_string.h"

const uint8_t ProxyErr::ID = 6U;

const std::string ProxyErr::reasonText[PROXY_ERR_NREASON] = {
    "Response is empty",
    "Verify DH keys failed",
    "Invalid hub address",
    "HTTP disconnected",
};

const std::string ProxyErr::funcName[PROXY_ERR_NFUNC] = {
    "Proxy::exchangeKey",
    "Proxy::registerConnection",
    "Proxy::post",
};

const std::string& ProxyErr::getReason(uint8_t reasonCode) noexcept {
    if (reasonCode > PROXY_ERR_NREASON) {
        return "UNKNOWN";
    }
    return ProxyErr::reasonText[reasonCode - 1];
}

const std::string& ProxyErr::getFunctionName(uint8_t funcID) noexcept {
    if (funcID > PROXY_ERR_NFUNC) {
        return "UNKNOWN";
    }
    return ProxyErr::funcName[funcID - 1];
}

std::string ProxyErr::getString(uint8_t funcID, int32_t reasonCode, uint8_t extID) {
    auto funcName = ProxyErr::getFunctionName(funcID);
    if (extID == External::Nil) {
        return UtilsString::format("[%s]:%s", funcName.c_str(), ProxyErr::getReason(reasonCode).c_str());
    }
    return UtilsString::format("[%s]:%s", funcName.c_str(), External::getReason(extID, reasonCode).c_str());
}