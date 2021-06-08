extern "C" {
    #include <mbedtls/sha256.h>
}
#include <stdexcept>
#include <esp_system.h>
#include "message/define.h"
#include "utils/utils_dh.h"


BigNum UtilsDH::generatePrivateKey() {
    return BigNum(abs((int32_t)esp_random()));
}

BigNum UtilsDH::calcPublicKey(const BigNum& gKey, const BigNum& nKey, const BigNum& privKey) {
    return gKey.powMod(privKey, nKey);
}

void UtilsDH::calcSecretKey(const BigNum& nKey, const BigNum& clientPrivKey, const BigNum& serverPubKey, uint8_t outSecretKey[32]) {
    std::string sharedKey = serverPubKey.powMod(clientPrivKey, nKey).toString();
    mbedtls_sha256((uint8_t*)sharedKey.c_str(), sharedKey.length(), outSecretKey, 0);
}
