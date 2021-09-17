extern "C" {
    #include <mbedtls/sha256.h>
}
#include <esp_system.h>
#include "message/define.h"
#include "utils/utils_dh.h"

std::tuple<Error, BigInt> UtilsDH::generatePrivateKey() {
    BigInt result;
    auto err = result.setNumber(abs((int32_t)esp_random()));
    return std::make_tuple(std::move(err), std::move(result));
}

std::tuple<Error, BigInt> UtilsDH::calcPublicKey(const BigInt& gKey, const BigInt& nKey, const BigInt& privKey) {
    return gKey.powMod(privKey, nKey);
}

std::tuple<Error, Array<uint8_t>> UtilsDH::calcSecretKey(const BigInt& nKey, const BigInt& clientPrivKey, const BigInt& serverPubKey) {
    // Calculate secret key and convert to string
    std::string str;
    {
        Error err;
        BigInt secretKey;

        std::tie(err, secretKey) = serverPubKey.powMod(clientPrivKey, nKey);
        if (!err.nil()) {
            return std::make_tuple(std::move(err), Array<uint8_t>{});
        }

        std::tie(err, str) = secretKey.toString();
        if (!err.nil()) {
            return std::make_tuple(std::move(err), Array<uint8_t>{});
        }
    }

    Array<uint8_t> secretKey{ 32 };
    mbedtls_sha256((uint8_t*)str.c_str(), str.length(), secretKey.get(), 0);
    return std::make_tuple(Error{}, std::move(secretKey));
}
