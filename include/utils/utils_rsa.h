#ifndef UTILS_RSA_H
#define UTILS_RSA_H

#include <cstdint>
#include "entity/array.h"
#include "error/error.h"

class UtilsRSA {
public:
    static std::tuple<Error::Code, bool> verifySignature(const std::string& publicKey, const Array<uint8_t>& signature, const Array<uint8_t>& data);
};

#endif //!UTILS_RSA_H
