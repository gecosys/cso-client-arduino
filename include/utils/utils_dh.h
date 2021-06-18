#ifndef _UTILS_DH_H_
#define _UTILS_DH_H_

#include "utils/result.h"
#include "utils/bignum.h"
#include "error/error_code.h"

class UtilsDH {
public:
    static Result<BigNum> generatePrivateKey();
    // "nkey" must be odd because "BigNum" does not support mod for even number
    static Result<BigNum> calcPublicKey(const BigNum& gKey, const BigNum& nKey, const BigNum& privKey);
    // "nkey" must be odd because "BigNum" does not support mod for even number
    static Error::Code calcSecretKey(const BigNum& nKey, const BigNum& clientPrivKey, const BigNum& serverPubKey, uint8_t outSecretKey[32]);
};

#endif
