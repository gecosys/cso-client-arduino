extern "C" {
    #include <mbedtls/gcm.h>
    #include <mbedtls/aes.h>
}
#include <esp_system.h>
#include "utils/utils_aes.h"


Error::Code UtilsAES::encrypt(const uint8_t key[32], const uint8_t* input, uint16_t sizeInput, const uint8_t* aad, uint8_t sizeAad, uint8_t outIV[LENGTH_IV], uint8_t outAuthenTag[LENGTH_AUTHEN_TAG], uint8_t* output) {
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

    auto errorCode = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES , key, 256);
    if (errorCode != 0) {
        mbedtls_gcm_free(&ctx);
        return Error::adaptExternalCode(ExternalTag::MbedTLS, errorCode);
    }

    esp_fill_random(outIV, LENGTH_IV);
    errorCode = mbedtls_gcm_crypt_and_tag(
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

    if (errorCode != 0) {
        return Error::adaptExternalCode(ExternalTag::MbedTLS, errorCode);
    }
    return Error::Nil;
}

Error::Code UtilsAES::decrypt(const uint8_t key[32], const uint8_t* input, uint16_t sizeInput, const uint8_t* aad, uint8_t sizeAad, const uint8_t iv[LENGTH_IV], const uint8_t authenTag[LENGTH_AUTHEN_TAG], uint8_t* output) {
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);

    auto errorCode = mbedtls_gcm_setkey(&ctx,MBEDTLS_CIPHER_ID_AES , key, 256);
    if (errorCode != 0) {
        mbedtls_gcm_free(&ctx);
        return Error::adaptExternalCode(ExternalTag::MbedTLS, errorCode);
    }

    errorCode = mbedtls_gcm_auth_decrypt(
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

    if (errorCode != 0) {
        return Error::adaptExternalCode(ExternalTag::MbedTLS, errorCode);
    }
    return Error::Nil;
}