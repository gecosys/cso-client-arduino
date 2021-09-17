#ifndef UTILS_HMAC_H
#define UTILS_HMAC_H

#include <tuple>
#include <cstdint>
#include "error/error.h"
#include "entity/array.hpp"

class UtilsHMAC {
public:
    static std::tuple<Error, Array<uint8_t>> calcHMAC(const Array<uint8_t>& key, const Array<uint8_t>& data);
    static std::tuple<Error, bool> validateHMAC(const Array<uint8_t>& key, const Array<uint8_t>& data, const Array<uint8_t>& expectedHMAC);
};

#endif // !UTILS_HMAC_H