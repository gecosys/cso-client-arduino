#ifndef _UTILS_AES_H_
#define _UTILS_AES_H_

#include <cstdint>
#include "message/define.h"
#include "error/error_code.h"

class UtilsAES {
public:
    static Error::Code encrypt(const uint8_t key[32], const uint8_t* input, uint16_t sizeInput, const uint8_t* aad, uint8_t sizeAad, uint8_t outIV[LENGTH_IV], uint8_t outAuthenTag[LENGTH_AUTHEN_TAG], uint8_t* output);
    static Error::Code decrypt(const uint8_t key[32], const uint8_t* input, uint16_t sizeInput, const uint8_t* aad, uint8_t sizeAad, const uint8_t iv[LENGTH_IV], const uint8_t authenTag[LENGTH_AUTHEN_TAG], uint8_t* output);
};

#endif
