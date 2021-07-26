#ifndef ERROR_PARSER_H
#define ERROR_PARSER_H

#include <string>
#include <cstdint>

#define PARSER_ERR_NREASON 1
#define PARSER_ERR_NFUNC   1

class ParserErr {
public:
    //=============
    // Reason codes
    //=============
    enum Reason : uint8_t {
        HMAC_Invalid = 1U,
    };

    //=============
    // Function IDs
    //=============
    enum Func : uint8_t {
        Parser_ParseReceiveMsg = 1U,
    };

private:
    static const std::string reasonText[PARSER_ERR_NREASON];
    static const std::string funcName[PARSER_ERR_NFUNC];

private:
    static const std::string& getReason(uint8_t reasonCode) noexcept;
    static const std::string& getFunctionName(uint8_t funcID) noexcept;

public:
    static const uint8_t ID;
    static std::string getString(uint8_t funcID, int32_t reasonCode, uint8_t extID);
};

#endif // !ERROR_PARSER_H