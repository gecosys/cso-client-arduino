#include <math.h>
#include "error/error.h"
#include "error/package/connection_err.h"
#include "error/package/connector_err.h"
#include "error/package/entity_err.h"
#include "error/package/message_err.h"
#include "error/package/parser_err.h"
#include "error/package/proxy_err.h"
#include "error/package/utils_err.h"

std::tuple<uint8_t, uint8_t, uint8_t, uint32_t> Error::parseCode(Error::Code code) noexcept {
    return std::make_tuple(
        (code >> 48ULL),
        (code >> 40ULL),
        (code >> 32ULL),
        (code & 0xFFFFFFFFULL)
    );
}

Error::Code Error::buildCode(uint8_t packID, uint8_t funcID, int32_t code, uint8_t extID) noexcept {
    uint64_t ret = code < 0 ? -code : code;
    ret |= (uint64_t(packID) << 48ULL);
    ret |= (uint64_t(funcID) << 40ULL);
    ret |= (uint64_t(extID)  << 32ULL);
    return (Error::Code)ret;
}

std::string Error::getString(Error::Code code) noexcept {
    uint8_t packID;
    uint8_t funcID;
    uint8_t extID;
    uint32_t reasonCode;

    std::tie(packID, funcID, extID, reasonCode) = Error::parseCode(code);

    //===================
    // Connection package
    //===================
    if (packID == ConnectionErr::ID) {
        return ConnectionErr::getString(funcID, reasonCode, extID);
    }

    //==================
    // Connector package
    //==================
    if (packID == ConnectorErr::ID) {
        return ConnectorErr::getString(funcID, reasonCode, extID);
    }

    //===============
    // Entity package
    //===============
    if (packID == EntityErr::ID) {
        return EntityErr::getString(funcID, reasonCode, extID);
    }

    //================
    // Message package
    //================
    if (packID == MessageErr::ID) {
        return MessageErr::getString(funcID, reasonCode, extID);
    }

    //===============
    // Parser package
    //===============
    if (packID == ParserErr::ID) {
        ParserErr::getString(funcID, reasonCode, extID);
    }

    //==============
    // Proxy package
    //==============
    if (packID == ProxyErr::ID) {
        return ProxyErr::getString(funcID, reasonCode, extID);
    }

    //==============
    // Utils package
    //==============
    if (packID == UtilsErr::ID) {
        return UtilsErr::getString(funcID, reasonCode, extID);
    }
    return "UNKNOWN ERROR";
}