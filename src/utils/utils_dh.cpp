extern "C" {
    #include <mbedtls/sha256.h>
}
#include <esp_system.h>
#include "message/define.h"
#include "utils/utils_dh.h"

std::tuple<Error::Code, BigInt> UtilsDH::generatePrivateKey() {
    BigInt result;
    auto errorCode = result.setNumber(abs((int32_t)esp_random()));
    return std::make_tuple(errorCode, std::move(result));
}

std::tuple<Error::Code, BigInt> UtilsDH::calcPublicKey(const BigInt& gKey, const BigInt& nKey, const BigInt& privKey) {
    return gKey.powMod(privKey, nKey);
}

std::tuple<Error::Code, Array<uint8_t>> UtilsDH::calcSecretKey(const BigInt& nKey, const BigInt& clientPrivKey, const BigInt& serverPubKey) {
    // Calculate secret key and convert to string
    std::string str;
    {
        Error::Code errcode;
        BigInt secretKey;

        std::tie(errcode, secretKey) = serverPubKey.powMod(clientPrivKey, nKey);
        if (errcode != Error::Code::Nil) {
            return std::make_tuple(errcode, Array<uint8_t>{});
        }

        std::tie(errcode, str) = secretKey.toString();
        if (errcode != Error::Code::Nil) {
            return std::make_tuple(errcode, Array<uint8_t>{});
        }
    }

    Array<uint8_t> secretKey{ 32 };
    mbedtls_sha256((uint8_t*)str.c_str(), str.length(), secretKey.get(), 0);
    return std::make_tuple(Error::Code::Nil, std::move(secretKey));
}
