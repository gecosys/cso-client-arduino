#ifndef UTILS_RSA_H
#define UTILS_RSA_H

#include <cstdint>
#include "error/error.h"
#include "entity/array.hpp"

class UtilsRSA {
public:
    static std::tuple<Error, bool> verifySignature(const std::string& publicKey, const Array<uint8_t>& signature, const Array<uint8_t>& data);
};

#endif //!UTILS_RSA_H
