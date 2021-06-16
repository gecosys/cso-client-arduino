extern "C" {
    #include <mbedtls/sha256.h>
}
#include <esp_system.h>
#include "message/define.h"
#include "utils/utils_dh.h"


Result<BigNum> UtilsDH::generatePrivateKey() {
    BigNum result;
    auto errorCode = result.setNumber(abs((int32_t)esp_random()));
    return make_result(errorCode, std::move(result));
}

Result<BigNum> UtilsDH::calcPublicKey(const BigNum& gKey, const BigNum& nKey, const BigNum& privKey) {
    return gKey.powMod(privKey, nKey);
}

Error::Code UtilsDH::calcSecretKey(const BigNum& nKey, const BigNum& clientPrivKey, const BigNum& serverPubKey, uint8_t outSecretKey[32]) {
    auto result_powmod = serverPubKey.powMod(clientPrivKey, nKey);
    if (result_powmod.errorCode != Error::Nil) {
        return result_powmod.errorCode;
    }

    auto result_tostr = result_powmod.data.toString();
    if (result_tostr.errorCode != Error::Nil) {
        return result_tostr.errorCode;
    }

    mbedtls_sha256((uint8_t*)result_tostr.data.c_str(), result_tostr.data.length(), outSecretKey, 0);
    return Error::Nil;
}
