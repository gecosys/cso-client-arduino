#ifndef _UTILS_AES_H_
#define _UTILS_AES_H_

#include <stdint.h>
#include <message/define.h>

class UtilsAES {
public:
    static void parseError(int errorCode, char *buffer, uint16_t buffLen);

    static int encrypt(const uint8_t key[32], const uint8_t *input, uint16_t sizeInput, const uint8_t *aad, uint8_t sizeAad, uint8_t *outIV, uint8_t *outAuthenTag, uint8_t *output);

    static int decrypt(const uint8_t key[32], const uint8_t *input, uint16_t sizeInput, const uint8_t *aad, uint8_t sizeAad, const uint8_t iv[LENGTH_IV], const uint8_t authenTag[LENGTH_AUTHEN_TAG], uint8_t *output);
};

#endif
