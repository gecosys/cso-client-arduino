#ifndef _UTILS_DH_H_
#define _UTILS_DH_H_

#include "utils/bignum.h"

class UtilsDH {
public:
    static BigNum generatePrivateKey();
    // "nkey" must be odd because "BigNum" does not support mod for even number
    static BigNum calcPublicKey(const BigNum& gKey, const BigNum& nKey, const BigNum& privKey);
    // "nkey" must be odd because "BigNum" does not support mod for even number
    static void calcSecretKey(const BigNum& nKey, const BigNum& clientPrivKey, const BigNum& serverPubKey, uint8_t outSecretKey[32]);
};

#endif
