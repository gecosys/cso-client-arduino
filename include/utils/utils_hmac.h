#ifndef _UTILS_HMAC_H_
#define _UTILS_HMAC_H_

#include <stdint.h>

class UtilsHMAC {
public:
    static void parseError(int errorCode, char *buffer, uint16_t buffLen);
    static int calcHMAC(const uint8_t key[32], const uint8_t *data, uint16_t sizeData, uint8_t outHMAC[32]);
    static bool validateHMAC(const uint8_t *key, const uint8_t *data, uint16_t sizeData, const uint8_t expectedHMAC[32]);
};

#endif
