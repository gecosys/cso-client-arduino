#include <utils/utils_rsa.h>

extern "C" {
    #include "mbedtls/pem.h"
    #include "mbedtls/pk.h"
    #include "mbedtls/sha256.h"
    #include "mbedtls/error.h"
}

void UtilsRSA::parseError(int errorCode, char *buffer, uint16_t buffLen) {
    mbedtls_strerror(errorCode, buffer, buffLen);
}

int UtilsRSA::verifySignature(unsigned char *publicKey, uint8_t sign[LENGTH_SIGN_RSA], uint8_t *data, uint16_t sizeData) {
    unsigned char hashed[32];
    mbedtls_sha256((unsigned char *)data, sizeData, hashed, 0);
    
    // Parse PEM
    size_t usedLen;
    mbedtls_pem_context pemCtx;
    mbedtls_pem_init(&pemCtx);
    auto errCode = mbedtls_pem_read_buffer(
        &pemCtx,
        "-----BEGIN PUBLIC KEY-----",
        "-----END PUBLIC KEY-----",
        publicKey,
        NULL,
        0,
        &usedLen
    );
    if (errCode != 0) {
        mbedtls_pem_free(&pemCtx);
        return errCode;
    }

    // Parse public key
    mbedtls_pk_context pkCtx;
    mbedtls_pk_init(&pkCtx);
    errCode = mbedtls_pk_parse_public_key(&pkCtx, pemCtx.buf, pemCtx.buflen);
    if (errCode != 0) {
        mbedtls_pem_free(&pemCtx);
        mbedtls_pk_free(&pkCtx);
        return errCode;
    }

    // Verify hashed data with signature
    errCode = mbedtls_pk_verify(
        &pkCtx,
        MBEDTLS_MD_SHA256,
        hashed,
        32,
        sign,
        256
    );
    mbedtls_pem_free(&pemCtx);
    mbedtls_pk_free(&pkCtx);
    if (errCode != 0) {
        return errCode;
    }
    return SUCCESS;
}