#ifndef ERROR_CONNECTION_H
#define ERROR_CONNECTION_H

#include <string>
#include <cstdint>

#define CONNECTION_ERR_NREASON 2
#define CONNECTION_ERR_NFUNC   4

class ConnectionErr {
public:
    //=============
    // Reason codes
    //=============
    enum Reason : uint8_t {
        Socket_ConnectFailed = 1U,
        Socket_Disconnected  = 2U,
    };

    //=============
    // Function IDs
    //=============
    enum Func : uint8_t {
        Connection_Connect		= 1U,
        Connection_LoopListen	= 2U,
        Connection_SendMessage  = 3U,
        Connection_Setup		= 4U,
    };

private:
    static const std::string reasonText[CONNECTION_ERR_NREASON];
    static const std::string funcName[CONNECTION_ERR_NFUNC];

private:
    static const std::string& getReason(uint8_t reasonCode) noexcept;
    static const std::string& getFunctionName(uint8_t funcID) noexcept;

public:
    static const uint8_t ID;
    static std::string getString(uint8_t funcID, int32_t reasonCode, uint8_t extID);
};

#endif // !ERROR_CONNECTION_H