extern "C" {
    #include <mbedtls/pk.h>
    #include <mbedtls/pem.h>
    #include <mbedtls/sha256.h>
}
#include "message/define.h"
#include "utils/utils_rsa.h"
#include "error/external.h"
#include "error/package/utils_err.h"


std::tuple<Error::Code, bool> UtilsRSA::verifySignature(const std::string& publicKey, const Array<uint8_t>& signature, const Array<uint8_t>& data) {
    uint8_t hashed[32];
    size_t usedLen;
    mbedtls_pem_context pemCtx;
    mbedtls_pk_context pkCtx;

    // Parse PEM
    mbedtls_pem_init(&pemCtx);
    auto errcode = mbedtls_pem_read_buffer(
        &pemCtx,
        "-----BEGIN PUBLIC KEY-----",
        "-----END PUBLIC KEY-----",
        (const uint8_t*)publicKey.c_str(),
        nullptr,
        0,
        &usedLen
    );
    if (errcode != 0) {
        goto handleError;
    }

    // Parse public key
    mbedtls_pk_init(&pkCtx);
    errcode = mbedtls_pk_parse_public_key(&pkCtx, pemCtx.buf, pemCtx.buflen);
    if (errcode != 0) {
        goto handleError;
    }

    // Verify hashed data with signature
    mbedtls_sha256(data.get(), data.length(), hashed, 0);
    errcode = mbedtls_pk_verify(
        &pkCtx,
        MBEDTLS_MD_SHA256,
        hashed,
        32,
        signature.get(),
        signature.length()
    );
    if (errcode != 0) {
        goto handleError;
    }    

    mbedtls_pem_free(&pemCtx);
    mbedtls_pk_free(&pkCtx);
    return std::make_tuple(Error::Code::Nil, true);

handleError:
    mbedtls_pem_free(&pemCtx);
    mbedtls_pk_free(&pkCtx);

    return std::make_tuple(
        Error::buildCode(
            UtilsErr::ID,
            UtilsErr::Func::RSA_VerifySignature,
            errcode,
            External::ID::MbedTLS
        ),
        false
    );
}