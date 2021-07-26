extern "C" {
    #include <mbedtls/gcm.h>
    #include <mbedtls/aes.h>
}
#include <esp_system.h>
#include "message/define.h"
#include "utils/utils_aes.h"
#include "error/external.h"
#include "error/package/utils_err.h"

std::tuple<Error::Code, Array<uint8_t>, Array<uint8_t>, Array<uint8_t>> UtilsAES::encrypt(const Array<uint8_t>& key, const Array<uint8_t>& input, const Array<uint8_t>& aad) {
    if (key.length() != LENGTH_KEY) {
        return std::make_tuple(
            Error::buildCode(UtilsErr::ID, UtilsErr::Func::AES_Encrypt, UtilsErr::Reason::InvalidKeyLength),
            Array<uint8_t>{},
            Array<uint8_t>{},
            Array<uint8_t>{}
        );
    }
    
    int32_t errcode;
    Array<uint8_t> iv;
    Array<uint8_t> tag;
    Array<uint8_t> output;
    mbedtls_gcm_context ctx;

    mbedtls_gcm_init(&ctx);
    errcode = mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES , key.get(), 256);
    if (errcode != 0) {
        goto handleError;
    }

    iv.reset(LENGTH_IV);
    esp_fill_random(iv.get(), LENGTH_IV);

    tag.reset(LENGTH_AUTHEN_TAG);
    output.reset(input.length());

    errcode = mbedtls_gcm_crypt_and_tag(
        &ctx,
        MBEDTLS_GCM_ENCRYPT,
        input.length(),
        iv.get(),
        LENGTH_IV,
        aad.get(),
        aad.length(),
        input.get(),
        output.get(),
        LENGTH_AUTHEN_TAG,
        tag.get()
    );
    if (errcode != 0) {
        goto handleError;
    }

    mbedtls_gcm_free(&ctx);
    return std::make_tuple(
        Error::Code::Nil,
        std::move(iv),
        std::move(tag),
        std::move(output)
    );

handleError:
    mbedtls_gcm_free(&ctx);
    return std::make_tuple(
        Error::buildCode(
            UtilsErr::ID, 
            UtilsErr::Func::AES_Encrypt, 
            errcode, 
            External::ID::MbedTLS
        ),
        Array<uint8_t>{},
        Array<uint8_t>{},
        Array<uint8_t>{}
    );
}

std::tuple<Error::Code, Array<uint8_t>> UtilsAES::decrypt(const Array<uint8_t>& key, const Array<uint8_t>& input, const Array<uint8_t>& aad, const Array<uint8_t>& iv, const Array<uint8_t>& tag) {
    if (iv.length() != LENGTH_IV || tag.length() != LENGTH_AUTHEN_TAG) {
        return std::make_tuple(
            Error::buildCode(UtilsErr::ID, UtilsErr::Func::AES_Decrypt, UtilsErr::Reason::AES_InvalidInput),
            Array<uint8_t>{}
        );
    }
    
    if (key.length() != LENGTH_KEY) {
        return std::make_tuple(
            Error::buildCode(UtilsErr::ID, UtilsErr::Func::AES_Decrypt, UtilsErr::Reason::InvalidKeyLength),
            Array<uint8_t>{}
        );
    }
    
    Array<uint8_t> output;
    mbedtls_gcm_context ctx;

    mbedtls_gcm_init(&ctx);
    auto errcode = mbedtls_gcm_setkey(&ctx,MBEDTLS_CIPHER_ID_AES , key.get(), 256);
    if (errcode != 0) {
        goto handleError;
    }

    output.reset(input.length());
    errcode = mbedtls_gcm_auth_decrypt(
        &ctx, 
        input.length(),
        iv.get(),
        LENGTH_IV,
        aad.get(),
        aad.length(),
        tag.get(),
        LENGTH_AUTHEN_TAG,
        input.get(),
        output.get()
    );
    if (errcode != 0) {
        goto handleError;
    
    }
    mbedtls_gcm_free(&ctx);
    return std::make_tuple(Error::Code::Nil, output);

handleError:
    mbedtls_gcm_free(&ctx);
    return std::make_tuple(
        Error::buildCode(
            UtilsErr::ID, 
            UtilsErr::Func::AES_Decrypt, 
            errcode, 
            External::ID::MbedTLS
        ),
        Array<uint8_t>{}
    );
}