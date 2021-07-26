#ifndef ERROR_PROXY_H
#define ERROR_PROXY_H

#include <string>
#include <cstdint>

#define PROXY_ERR_NREASON 4
#define PROXY_ERR_NFUNC	  3

class ProxyErr {
public:
    //=============
    // Reason codes
    //=============
    enum Reason : uint8_t {
        Response_Empty	    = 1U,
        DHKeys_VerifyFailed = 2U,
        HubAddress_Invalid	= 3U,
        Http_Disconnected   = 4U,
    };

    //=============
    // Function IDs
    //=============
    enum Func : uint8_t {
        Proxy_ExchangeKey		 = 1U,
        Proxy_RegisterConnection = 2U,
        Proxy_Post               = 3U,
    };

private:
    static const std::string reasonText[PROXY_ERR_NREASON];
    static const std::string funcName[PROXY_ERR_NFUNC];

private:
    static const std::string& getReason(uint8_t reasonCode) noexcept;
    static const std::string& getFunctionName(uint8_t funcID) noexcept;

public:
    static const uint8_t ID;
    static std::string getString(uint8_t funcID, int32_t reasonCode, uint8_t extID);
};

#endif // !ERROR_PROXY_H