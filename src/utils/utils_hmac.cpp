#include <utils/utils_hmac.h>
#include <message/define.h>
#include <string.h>

extern "C" {
    #include "mbedtls/error.h"
    #include "mbedtls/md.h"
    #include "mbedtls/sha256.h"
}

void UtilsHMAC::parseError(int errorCode, char *buffer, uint16_t buffLen) {
    mbedtls_strerror(errorCode, buffer, buffLen);
}

int UtilsHMAC::calcHMAC(const uint8_t key[32], const uint8_t *data, uint16_t sizeData, uint8_t outHMAC[32]) {
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    mbedtls_md_init(&ctx);

    int errCode = mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    if (errCode != 0) {
        return errCode;
    }

    mbedtls_md_hmac_starts(&ctx, (const unsigned char *) key, 32);
    if (errCode != 0) {
        return errCode;
    }

    mbedtls_md_hmac_update(&ctx, data, sizeData);
    if (errCode != 0) {
        return errCode;
    }

    mbedtls_md_hmac_finish(&ctx, outHMAC);
    if (errCode != 0) {
        return errCode;
    }

    mbedtls_md_free(&ctx);
    return SUCCESS;
}

bool UtilsHMAC::validateHMAC(const uint8_t *key, const uint8_t *data, uint16_t sizeData, const uint8_t expectedHMAC[32]) {
    uint8_t hmac[32];
    int errCode = UtilsHMAC::calcHMAC(key, data, sizeData, hmac);
    if (errCode != 0) {
        return false;
    }
    return memcmp(hmac, expectedHMAC, 32) == 0;
}
