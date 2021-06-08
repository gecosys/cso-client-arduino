extern "C" {
    #include <mbedtls/pk.h>
    #include <mbedtls/pem.h>
    #include <mbedtls/sha256.h>
}
#include "utils/utils_rsa.h"


Error::Code UtilsRSA::verifySignature(const uint8_t* publicKey, const uint8_t* sign, uint16_t sizeSign, const uint8_t* data, uint16_t sizeData) {
    uint8_t hashed[32];
    mbedtls_sha256(data, sizeData, hashed, 0);

    // Parse PEM
    size_t usedLen;
    mbedtls_pem_context pemCtx;
    mbedtls_pem_init(&pemCtx);
    int32_t error = mbedtls_pem_read_buffer(
        &pemCtx,
        "-----BEGIN PUBLIC KEY-----",
        "-----END PUBLIC KEY-----",
        publicKey,
        NULL,
        0,
        &usedLen
    );
    if (error != 0) {
        mbedtls_pem_free(&pemCtx);
        return Error::adaptExternalCode(ExtTag::MbedTLS, error);
    }

    // Parse public key
    mbedtls_pk_context pkCtx;
    mbedtls_pk_init(&pkCtx);
    error = mbedtls_pk_parse_public_key(&pkCtx, pemCtx.buf, pemCtx.buflen);
    if (error != 0) {
        mbedtls_pem_free(&pemCtx);
        mbedtls_pk_free(&pkCtx);
        return Error::adaptExternalCode(ExtTag::MbedTLS, error);
    }

    // Verify hashed data with signature
    error = mbedtls_pk_verify(
        &pkCtx,
        MBEDTLS_MD_SHA256,
        hashed,
        32,
        sign,
        sizeSign
    );
    mbedtls_pem_free(&pemCtx);
    mbedtls_pk_free(&pkCtx);
    if (error != 0) {
        return Error::adaptExternalCode(ExtTag::MbedTLS, error);
    }    
    return Error::Nil;
}