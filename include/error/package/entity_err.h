#ifndef ERROR_ENTITY_H
#define ERROR_ENTITY_H

#include <string>
#include <cstdint>

#define ENTITY_ERR_NREASON 2
#define ENTITY_ERR_NFUNC   7

class EntityErr {
public:
    //=============
    // Reason codes
    //=============
    enum Reason : uint8_t {
        SPSCQueue_QueueFull  = 1U,
        SPSCQueue_QueueEmpty = 2U,
    };

    //=============
    // Function IDs
    //=============
    enum Func : uint8_t {
        BigInt_Copy       = 1U,
        BigInt_SetNumber  = 2U,
        BigInt_SetString  = 3U,
        BigInt_Powmod     = 4U,
        BigInt_ToString   = 5U,
        SPSCQueue_TryPush = 6U,
        SPSCQueue_TryPop  = 7U,
    };

private:
    static const std::string reasonText[ENTITY_ERR_NREASON];
    static const std::string funcName[ENTITY_ERR_NFUNC];

private:
    static const std::string& getReason(uint8_t reasonCode) noexcept;
    static const std::string& getFunctionName(uint8_t funcID) noexcept;

public:
    static const uint8_t ID;
    static std::string getString(uint8_t funcID, int32_t reasonCode, uint8_t extID);
};

#endif // !ERROR_ENTITY_H