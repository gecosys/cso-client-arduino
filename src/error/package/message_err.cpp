#include "error/external.h"
#include "error/package/message_err.h"
#include "utils/utils_string.h"

const uint8_t MessageErr::ID = 4U;

const std::string MessageErr::reasonText[MSG_ERR_NREASON] = {
    "Length of ticket must be 34",
    "Length of token must be 32",
    "Length of ready ticket must be 21",
    "Length of name doesn't be over 36",
    "Invalid buffer length",
    "Length of 'iv' must be 'LENGTH_IV' be defined in 'message/define.h'",
    "Length of 'authenTag' must be 'LENGTH_AUTHEN_TAG' be defined in 'message/define.h'",
    "Length of 'sign' must be 'LENGTH_SIGN' be defined in 'message/define.h'",
};

const std::string MessageErr::funcName[MSG_ERR_NFUNC] = {
    "Ticket::parseBytes",
    "ReadyTicket::parseBytes",
    "Cipher::setIV",
    "Cipher::setAuthenTag",
    "Cipher::setSign",
    "Cipher::parseBytes",
    "Cipher::buildRawBytes",
    "Cipher::buildAad",
    "Cipher::buildCipherBytes",
    "Cipher::buildNoCipherBytes",
};

const std::string& MessageErr::getReason(uint8_t reasonCode) noexcept {
    if (reasonCode > MSG_ERR_NREASON) {
        return "UNKNOWN";
    }
    return MessageErr::reasonText[reasonCode - 1];
}

const std::string& MessageErr::getFunctionName(uint8_t funcID) noexcept {
    if (funcID > MSG_ERR_NFUNC) {
        return "UNKNOWN";
    }
    return MessageErr::funcName[funcID - 1];
}

std::string MessageErr::getString(uint8_t funcID, int32_t reasonCode, uint8_t extID) {
    auto funcName = MessageErr::getFunctionName(funcID);
    if (extID == External::Nil) {
        return UtilsString::format("[%s]:%s", funcName.c_str(), MessageErr::getReason(reasonCode).c_str());
    }
    return UtilsString::format("[%s]:%s", funcName.c_str(), External::getReason(extID, reasonCode).c_str());
}