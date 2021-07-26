#include "error/external.h"
#include "error/package/entity_err.h"
#include "utils/utils_string.h"

const uint8_t EntityErr::ID = 3U;

const std::string EntityErr::reasonText[ENTITY_ERR_NREASON] = {
    "Queue is full",
    "Queue is empty",
};

const std::string EntityErr::funcName[ENTITY_ERR_NFUNC] = {
    "BigInt::copy",
    "BigInt::setNumber",
    "BigInt::setString",
    "BigInt::powmod",
    "BigInt::toString",
    "SPSCQueue::try_push",
    "SPSCQueue::try_pop",
};

const std::string& EntityErr::getReason(uint8_t reasonCode) noexcept {
    if (reasonCode > ENTITY_ERR_NREASON) {
        return "UNKNOWN";
    }
    return EntityErr::reasonText[reasonCode - 1];
}

const std::string& EntityErr::getFunctionName(uint8_t funcID) noexcept {
    if (funcID > ENTITY_ERR_NFUNC) {
        return "UNKNOWN";
    }
    return EntityErr::funcName[funcID - 1];
}

std::string EntityErr::getString(uint8_t funcID, int32_t reasonCode, uint8_t extID) {
    auto funcName = EntityErr::getFunctionName(funcID);
    if (extID == External::Nil) {
        return UtilsString::format("[%s]:%s", funcName.c_str(), EntityErr::getReason(reasonCode).c_str());
    }
    return UtilsString::format("[%s]:%s", funcName.c_str(), External::getReason(extID, reasonCode).c_str());
}