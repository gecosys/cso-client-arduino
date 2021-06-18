extern "C" {
    #include <mbedtls/md.h>
    #include <mbedtls/sha256.h>
}
#include <cstring>
#include <esp32-hal-log.h>
#include "message/define.h"
#include "utils/utils_hmac.h"

Error::Code UtilsHMAC::calcHMAC(const uint8_t key[32], const uint8_t* data, uint16_t sizeData, uint8_t outHMAC[32]) {
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    mbedtls_md_init(&ctx);

    auto errorCode = mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    if (errorCode != 0) {
        return Error::adaptExternalCode(ExternalTag::MbedTLS, errorCode);
    }

    errorCode = mbedtls_md_hmac_starts(&ctx, (const unsigned char *) key, 32);
    if (errorCode != 0) {
        mbedtls_md_free(&ctx);
        return Error::adaptExternalCode(ExternalTag::MbedTLS, errorCode);
    }

    errorCode = mbedtls_md_hmac_update(&ctx, data, sizeData);
    if (errorCode != 0) {
        mbedtls_md_free(&ctx);
        return Error::adaptExternalCode(ExternalTag::MbedTLS, errorCode);
    }

    errorCode = mbedtls_md_hmac_finish(&ctx, outHMAC);
    mbedtls_md_free(&ctx);

    if (errorCode != 0) {
        return Error::adaptExternalCode(ExternalTag::MbedTLS, errorCode);
    }
    return Error::Nil;
}

bool UtilsHMAC::validateHMAC(const uint8_t* key, const uint8_t* data, uint16_t sizeData, const uint8_t expectedHMAC[32]) {
    uint8_t hmac[32];
    auto errorCode = UtilsHMAC::calcHMAC(key, data, sizeData, hmac);
    if (errorCode != Error::Nil) {
        log_e("%s", Error::getContent(errorCode));
        return false;
    }
    return memcmp(hmac, expectedHMAC, 32) == 0;
}
