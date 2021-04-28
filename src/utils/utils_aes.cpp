#include <utils/utils_aes.h>
#include <Arduino.h>

extern "C" {
    #include "mbedtls/error.h"
    #include "mbedtls/gcm.h"
    #include "mbedtls/aes.h"
}

void UtilsAES::parseError(int errorCode, char *buffer, uint16_t buffLen) {
    mbedtls_strerror(errorCode, buffer, buffLen);
}

int UtilsAES::encrypt(const uint8_t key[32], const uint8_t *input, uint16_t sizeInput, const uint8_t *aad, uint8_t sizeAad, uint8_t *outIV, uint8_t *outAuthenTag, uint8_t *output) {
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

    auto errCode = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES , key, 256);
    if (errCode != 0) {
        mbedtls_gcm_free(&ctx);
        return errCode;
    }

    esp_fill_random(outIV, LENGTH_IV);
    mbedtls_gcm_crypt_and_tag(
        &ctx,
        MBEDTLS_GCM_ENCRYPT,
        sizeInput,
        outIV,
        LENGTH_IV,
        aad,
        sizeAad,
        input,
        output,
        LENGTH_AUTHEN_TAG,
        outAuthenTag
    );

    mbedtls_gcm_free(&ctx);
    return SUCCESS;
}

int UtilsAES::decrypt(const uint8_t key[32], const uint8_t *input, uint16_t sizeInput, const uint8_t *aad, uint8_t sizeAad, const uint8_t iv[LENGTH_IV], const uint8_t authenTag[LENGTH_AUTHEN_TAG], uint8_t *output) {
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

    auto errCode = mbedtls_gcm_setkey(&ctx,MBEDTLS_CIPHER_ID_AES , key, 256);
    if (errCode != 0) {
        mbedtls_gcm_free(&ctx);
        return errCode;
    }

    mbedtls_gcm_auth_decrypt(
        &ctx, sizeInput,
        iv,
        LENGTH_IV,
        aad,
        sizeAad,
        authenTag,
        LENGTH_AUTHEN_TAG,
        input,
        output
    );

    mbedtls_gcm_free(&ctx);
    return SUCCESS;
}