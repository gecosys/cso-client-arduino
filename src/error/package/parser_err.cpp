#include "error/external.h"
#include "error/package/parser_err.h"
#include "utils/utils_string.h"

const uint8_t ParserErr::ID = 5U;

const std::string ParserErr::reasonText[PARSER_ERR_NREASON] = {
    "Validate HMAC failed",
};

const std::string ParserErr::funcName[PARSER_ERR_NFUNC] = {
    "Parser::parseReceivedMessage",
};

const std::string& ParserErr::getReason(uint8_t reasonCode) noexcept {
    if (reasonCode > PARSER_ERR_NREASON) {
        return "UNKNOWN";
    }
    return ParserErr::reasonText[reasonCode - 1];
}

const std::string& ParserErr::getFunctionName(uint8_t funcID) noexcept {
    if (funcID > PARSER_ERR_NFUNC) {
        return "UNKNOWN";
    }
    return ParserErr::funcName[funcID - 1];
}

std::string ParserErr::getString(uint8_t funcID, int32_t reasonCode, uint8_t extID) {
    auto funcName = ParserErr::getFunctionName(funcID);
    if (extID == External::Nil) {
        return UtilsString::format("[%s]:%s", funcName.c_str(), ParserErr::getReason(reasonCode).c_str());
    }
    return UtilsString::format("[%s]:%s", funcName.c_str(), External::getReason(extID, reasonCode).c_str());
}