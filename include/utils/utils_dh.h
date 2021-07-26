#ifndef UTILS_DH_H
#define UTILS_DH_H

#include <tuple>
#include "entity/array.h"
#include "entity/bigint.h"
#include "error/error.h"

class UtilsDH {
public:
    static std::tuple<Error::Code, BigInt> generatePrivateKey();
    // "nkey" must be odd because "BigInt" does not support mod for even number
    static std::tuple<Error::Code, BigInt> calcPublicKey(const BigInt& gKey, const BigInt& nKey, const BigInt& privKey);
    // "nkey" must be odd because "BigInt" does not support mod for even number
    static std::tuple<Error::Code, Array<uint8_t>> calcSecretKey(const BigInt& nKey, const BigInt& clientPrivKey, const BigInt& serverPubKey);
};

#endif // !UTILS_DH_H
