#ifndef ERROR_CONNECTOR_H
#define ERROR_CONNECTOR_H

#include <string>
#include <cstdint>

#define CONNECTOR_ERR_NREASON 2
#define CONNECTOR_ERR_NFUNC   4

class ConnectorErr {
public:
    //=============
    // Reason codes
    //=============
    enum Reason : uint8_t {
        Connection_NotActivatd = 1U,
        MsgQueue_Full          = 2U,
    };

    //=============
    // Function IDs
    //=============
    enum Func : uint8_t {
        Connector_SendMsg			= 1U,
        Connector_SendGroupMsg		= 2U,
        Connector_SendMsgRetry		= 3U,
        Connector_SendGroupMsgRetry = 4U,
    };

private:
    static const std::string reasonText[CONNECTOR_ERR_NREASON];
    static const std::string funcName[CONNECTOR_ERR_NFUNC];

private:
    static const std::string& getReason(uint8_t reasonCode) noexcept;
    static const std::string& getFunctionName(uint8_t funcID) noexcept;

public:
    static const uint8_t ID;
    static std::string getString(uint8_t funcID, int32_t reasonCode, uint8_t extID);
};

#endif // !ERROR_CONNECTOR_H