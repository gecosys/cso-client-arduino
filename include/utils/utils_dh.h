#ifndef _UTILS_DH_H_
#define _UTILS_DH_H_

#include <BigNumber.h>

class UtilsDH {
public:
    static BigNumber generatePrivateKey();
    static BigNumber calcPublicKey(BigNumber gKey, BigNumber nKey, BigNumber privKey);
    static void calcSecretKey(BigNumber nKey, BigNumber clientPrivKey, BigNumber serverPubKey, uint8_t outSecretKey[32]);
};

#endif
