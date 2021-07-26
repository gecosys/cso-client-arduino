#include "error/external.h"
#include "error/package/utils_err.h"
#include "utils/utils_string.h"

const uint8_t UtilsErr::ID = 7U;

const std::string UtilsErr::reasonText[UTILS_ERR_NREASON] = {
    "Length of iv or tag is not default value in 'message/define.h'",
    "Length of key must be 32",
};

const std::string UtilsErr::funcName[UTILS_ERR_NFUNC] = {
    "UtilsAES::encrypt",
    "UtilsAES::decrypt",
    "UtilsHMAC::calcHMAC",
    "UtilsDH::calcSecretKey",
    "UtilsRSA::verifySignature",
};

const std::string& UtilsErr::getReason(uint8_t reasonCode) noexcept {
    if (reasonCode > UTILS_ERR_NREASON) {
        return "UNKNOWN";
    }
    return UtilsErr::reasonText[reasonCode - 1];
}

const std::string& UtilsErr::getFunctionName(uint8_t funcID) noexcept {
    if (funcID > UTILS_ERR_NFUNC) {
        return "UNKNOWN";
    }
    return UtilsErr::funcName[funcID - 1];
}

std::string UtilsErr::getString(uint8_t funcID, int32_t reasonCode, uint8_t extID) {
    auto funcName = UtilsErr::getFunctionName(funcID);
    if (extID == External::Nil) {
        return UtilsString::format("[%s]:%s", funcName.c_str(), UtilsErr::getReason(reasonCode).c_str());
    }
    return UtilsString::format("[%s]:%s", funcName.c_str(), External::getReason(extID, reasonCode).c_str());
}