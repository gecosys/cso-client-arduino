#ifndef ERROR_UTILS_H
#define ERROR_UTILS_H

#include <string>
#include <cstdint>

#define UTILS_ERR_NREASON 2
#define UTILS_ERR_NFUNC   5

class UtilsErr {
public:
    //=============
    // Reason codes
    //=============
    enum Reason : uint8_t {
        AES_InvalidInput    = 1U,
        InvalidKeyLength    = 2U,
    };

    //=============
    // Function IDs
    //=============
    enum Func : uint8_t {
        AES_Encrypt			= 1U,
        AES_Decrypt			= 2U,
        HMAC_Calc			= 3U,
        DH_CalcSecretKey	= 4U,
        RSA_VerifySignature	= 5U,
    };

private:
    static const std::string reasonText[UTILS_ERR_NREASON];
    static const std::string funcName[UTILS_ERR_NFUNC];

private:
    static const std::string& getReason(uint8_t reasonCode) noexcept;
    static const std::string& getFunctionName(uint8_t funcID) noexcept;

public:
    static const uint8_t ID;
    static std::string getString(uint8_t funcID, int32_t reasonCode, uint8_t extID);
};

#endif // !ERROR_UTILS_H