#ifndef _UTILS_RSA_H_
#define _UTILS_RSA_H_

#include <cstdint>
#include "message/define.h"
#include "error/error_code.h"

class UtilsRSA {
public:
    static Error::Code verifySignature(const uint8_t* publicKey, const uint8_t* sign, uint16_t sizeSign, const uint8_t *data, uint16_t sizeData);
};

#endif
