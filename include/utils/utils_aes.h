#ifndef UTILS_AES_H
#define UTILS_AES_H

#include <tuple>
#include <cstdint>
#include "error/error.h"
#include "entity/array.h"

class UtilsAES {
public:
    // Result got order: error code, iv, tag, output
    // Output's length equals to input's length
    static std::tuple<Error::Code, Array<uint8_t>, Array<uint8_t>, Array<uint8_t>> encrypt(const Array<uint8_t>& key, const Array<uint8_t>& input, const Array<uint8_t>& aad);

    // Output's length equals to input's length
    static std::tuple<Error::Code, Array<uint8_t>> decrypt(const Array<uint8_t>& key, const Array<uint8_t>& input, const Array<uint8_t>& aad, const Array<uint8_t>& iv, const Array<uint8_t>& tag);
};

#endif // !UTILS_AES_H