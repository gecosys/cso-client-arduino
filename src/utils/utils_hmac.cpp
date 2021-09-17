extern "C" {
    #include <mbedtls/md.h>
    #include <mbedtls/sha256.h>
}
#include <cstring>
#include <esp32-hal-log.h>
#include "message/define.h"
#include "utils/utils_hmac.h"
#include "utils/utils_general.hpp"
#include "error/thirdparty.h"

std::tuple<Error, Array<uint8_t>> UtilsHMAC::calcHMAC(const Array<uint8_t>& key, const Array<uint8_t>& data) {
    if (key.length() != LENGTH_KEY) {
        return std::make_tuple(
            Error{ GET_FUNC_NAME(), "Length of key must be 32" },
            Array<uint8_t>{}
        );
    }
    
    int32_t errcode;
    Array<uint8_t> output;
    mbedtls_md_context_t ctx;

    mbedtls_md_init(&ctx);
    errcode = mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    if (errcode != 0) {
        goto handleError;
    }

    errcode = mbedtls_md_hmac_starts(&ctx, (const uint8_t*)key.get(), key.length());
    if (errcode != 0) {
        goto handleError;
    }

    errcode = mbedtls_md_hmac_update(&ctx, data.get(), data.length());
    if (errcode != 0) {
        goto handleError;
    }

    output.reset(key.length());
    errcode = mbedtls_md_hmac_finish(&ctx, output.get());
    if (errcode != 0) {
        goto handleError;
    }

    mbedtls_md_free(&ctx);
    return std::make_tuple(Error{}, std::move(output));

handleError:
    mbedtls_md_free(&ctx);
    return std::make_tuple(
        Error{ GET_FUNC_NAME(), Thirdparty::getMbedtlsError(errcode) },
        Array<uint8_t>{}
    );
}

std::tuple<Error, bool> UtilsHMAC::validateHMAC(const Array<uint8_t>& key, const Array<uint8_t>& data, const Array<uint8_t>& expectedHMAC) {
    Error err;
    Array<uint8_t> hmac;

    std::tie(err, hmac) = UtilsHMAC::calcHMAC(key, data);
    if (err.nil()) {
        return std::make_tuple(Error{}, memcmp(hmac.get() , expectedHMAC.get(), 32) == 0);
    }
    return std::make_tuple(std::move(err), false);
}
