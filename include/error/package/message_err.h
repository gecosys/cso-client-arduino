#ifndef ERROR_MESSAGE_H
#define ERROR_MESSAGE_H

#include <string>
#include <cstdint>

#define MSG_ERR_NREASON 8
#define MSG_ERR_NFUNC   10

class MessageErr {
public:
    //=============
    // Reason codes
    //=============
    enum Reason : uint8_t {
        Ticket_InvalidTicketSize	= 1U,
        Ticket_InvalidTokenSize     = 2U,
        ReadyTicket_InvalidSize		= 3U,
        Cipher_InvalidNameLen		= 4U,
        Cipher_InvalidBufferSize	= 5U,
        Cipher_InvalidIVLen			= 6U,
        Cipher_InvalidAuthenTagLen  = 7U,
        Cipher_InvalidSignLen		= 8U,
    };

    //=============
    // Function IDs
    //=============
    enum Func : uint8_t {
        Ticket_ParseBytes			= 1U,
        ReadyTicket_ParseBytes		= 2U,
        Cipher_SetIV				= 3U,
        Cipher_SetAuthenTag			= 4U,
        Cipher_SetSign				= 5U,
        Cipher_ParseBytes			= 6U,
        Cipher_BuildRawBytes		= 7U,
        Cipher_BuildAad				= 8U,
        Cipher_BuildCipherBytes		= 9U,
        Cipher_BuildNoCipherBytes	= 10U,
    };

private:
    static const std::string reasonText[MSG_ERR_NREASON];
    static const std::string funcName[MSG_ERR_NFUNC];

private:
    static const std::string& getReason(uint8_t reasonCode) noexcept;
    static const std::string& getFunctionName(uint8_t funcID) noexcept;

public:
    static const uint8_t ID;
    static std::string getString(uint8_t funcID, int32_t reasonCode, uint8_t extID);
};

#endif // !ERROR_MESSAGE_H