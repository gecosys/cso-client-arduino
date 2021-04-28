#include <utils/utils_dh.h>
#include <message/define.h>

extern "C" {
    #include "mbedtls/sha256.h"
}

BigNumber UtilsDH::generatePrivateKey() {
    return BigNumber(abs((int32_t)esp_random()));
}

BigNumber UtilsDH::calcPublicKey(BigNumber gKey, BigNumber nKey, BigNumber privKey) {
    return gKey.powMod(privKey, nKey);
}

void UtilsDH::calcSecretKey(BigNumber nKey, BigNumber clientPrivKey, BigNumber serverPubKey, uint8_t outSecretKey[32]) {
    auto sharedKey = serverPubKey.powMod(clientPrivKey, nKey).toString();
    mbedtls_sha256((unsigned char *)sharedKey, strlen(sharedKey), outSecretKey, 0);
    free(sharedKey);
}
