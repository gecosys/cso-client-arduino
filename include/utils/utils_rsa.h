#ifndef _UTILS_RSA_H_
#define _UTILS_RSA_H_

#include <stdint.h>
#include <message/define.h>

class UtilsRSA {
    static void parseError(int errorCode, char *buffer, uint16_t buffLen);
    static int verifySignature(unsigned char *publicKey, uint8_t sign[LENGTH_SIGN], uint8_t *data, uint16_t sizeData);
};

#endif
