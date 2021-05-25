#ifndef _UTILS_RSA_H_
#define _UTILS_RSA_H_

#include <stdint.h>
#include <message/define.h>

class UtilsRSA {
public:
    static void parseError(int errorCode, char *buffer, uint16_t buffLen);
    static int verifySignature(const unsigned char *publicKey, const uint8_t *sign, uint16_t sizeSign, const uint8_t *data, uint16_t sizeData);
};

#endif
